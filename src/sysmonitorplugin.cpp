#include "sysmonitorplugin.h"
#include "widgetplugin.hpp"
#include "mainwidget.h"
#include "tipswidget.h"
#include "pluginsettingdialog.h"
#include "aboutdialog.h"

#include <DDBusSender>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <cstdio>

constexpr char kPluginStateKey[] = "enable";

// 设置选项和默认值,静态全局变量
struct SettingItem SysMonitorPlugin::settingItems[] = {
    // 全局设置选项
    {"chartModeCheckBox", 0}, // 图表模式开关
    {"batInfoComboBox", 0}, // 气泡电池信息开关
    {"updateIntervalSpinBox", 1000}, // 更新间隔ms

    // 文字模式设置选项
    {"displayContentComboBox", DisplayContentSetting::ALL},
    {"wordSpacingSpinBox", 4},
    {"cpuDiyWordLineEdit", QString("CPU:")},
    {"memDiyWordLineEdit", QString("MEM:")},
    {"upNetspeedDiyWordLineEdit", QString("▴")},
    {"downNetspeedDiyWordLineEdit", QString("▾")},
    {"fontSizeSpinBox", 9},
    {"fontColorComboBox", 0},

    // 图表模式设置选项
    // 默认三个图表都是打开的
    {"netChartCheckBox", 1},
    {"cpuChartCheckBox", 1},
    {"memChartCheckBox", 1},
    {"chartSpacingSpinBox", 1},
    // net图表设置
    {"netUpTopSpinBox", 500}, // 上传
    {"netUpWidget", QColor(0, 78, 239, 200)},
    {"netDownTopSpinBox", 500}, // 下载
    {"netDownWidget", QColor(225, 67, 0, 200)},
    {"netBorderRoundSpinBox", 30}, // 边框
    {"netBorderWidget", QColor(255, 255, 255, 0)},
    {"netBackgroundWidget", QColor(0, 0, 0, 128)}, // 背景
    {"netWidthSpinBox", 40},
    {"netHeightSpinBox", 28}, // 宽度高度
    // cpu图表设置
    {"cpuWorkWidget", QColor(250, 74, 74, 255)},
    {"cpuBorderRoundSpinBox", 30}, // 边框
    {"cpuBorderWidget", QColor(255, 255, 255, 0)},
    {"cpuBackgroundWidget", QColor(0, 0, 0, 128)}, // 背景
    {"cpuWidthSpinBox", 40},
    {"cpuHeightSpinBox", 28}, // 宽度高度
    // mem图表设置
    {"memUsedWidget", QColor(21, 199, 195, 255)},
    {"memBorderRoundSpinBox", 30}, // 边框
    {"memBorderWidget", QColor(255, 255, 255, 0)},
    {"memBackgroundWidget", QColor(0, 0, 0, 128)}, // 背景
    {"memWidthSpinBox", 40},
    {"memHeightSpinBox", 28} // 宽度高度
};

SysMonitorPlugin::SysMonitorPlugin(QObject *parent)
    : QObject(parent)
    , m_refreshTimer(new QTimer(this))
{
    oldrbytes = oldsbytes = 0;
    oldworktime = oldtotaltime = 0;
}

SysMonitorPlugin::~SysMonitorPlugin()
{
    if (m_mainWidget) {
        m_mainWidget->deleteLater();
        m_mainWidget = nullptr;
    }

    if (m_tipsWidget) {
        m_tipsWidget->deleteLater();
        m_tipsWidget = nullptr;
    }
}

const QString SysMonitorPlugin::pluginDisplayName() const
{
    return QString("监视器");
}

const QString SysMonitorPlugin::pluginName() const
{
    return PLUGIN_NAME;
}

void SysMonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    PluginsItemInterface::m_proxyInter = proxyInter;
    m_proxyInter = new WidgetPlugin(PluginsItemInterface::m_proxyInter, this);

    // 读取显示配置
    readConfig(&settings);

    m_mainWidget = new MainWidget(settings, position());
    m_tipsWidget = new Dock::TipsWidget;
    font.setFamily("Noto Mono");
    m_tipsWidget->setFont(font);
    dismode = displayMode();
    pos = position();
    battery_watts = -1.0;
    bat_count = 0;

    // 如果插件没有被禁用则在初始化插件时才添加主控件到面板上
    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }

    // 设置 Timer 超时为 updateIntervalSpinBox
    // 中的ms，即每次更新一次控件上的数据，并启动这个定时器
    m_refreshTimer->start(settings.value("updateIntervalSpinBox").toInt());

    // 连接 Timer 超时的信号到更新数据的槽上
    connect(m_refreshTimer, &QTimer::timeout, this, &SysMonitorPlugin::refreshInfo, Qt::QueuedConnection);
}

QWidget *SysMonitorPlugin::itemWidget(const QString &itemKey)
{
    if (itemKey != pluginName()) {
        return nullptr;
    }

    return m_mainWidget;
}

QWidget *SysMonitorPlugin::itemTipsWidget(const QString &itemKey)
{
    if (itemKey != pluginName()) {
        return nullptr;
    }

    // 更新气泡数据
    updateWidget(m_tipsWidget);
    return m_tipsWidget;
}

QWidget *SysMonitorPlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey)

    return nullptr;
}

bool SysMonitorPlugin::pluginIsAllowDisable()
{
    // 告诉 dde-dock 本插件允许禁用
    return true;
}

bool SysMonitorPlugin::pluginIsDisable()
{
    // 第二个参数 “disabled”
    // 表示存储这个值的键（所有配置都是以键值对的方式存储的）
    // 第三个参数表示默认值，即默认不禁用
    return !m_proxyInter->getValue(this, kPluginStateKey, true).toBool();
}

void SysMonitorPlugin::pluginStateSwitched()
{
    // 获取当前禁用状态的反值作为新的状态值
    const bool disabledNew = !pluginIsDisable();
    // 存储新的状态值
    m_proxyInter->saveValue(this, kPluginStateKey, !disabledNew);

    // 根据新的禁用状态值处理主控件的加载和卸载
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

const QString SysMonitorPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey)

    QList<QVariant> items;
    items.reserve(4);

    QMap<QString, QVariant> refresh;
    refresh["itemId"] = "refresh";
    refresh["itemText"] = "刷新";
    refresh["isActive"] = true;
    items.push_back(refresh);

    QMap<QString, QVariant> open;
    open["itemId"] = "open";
    open["itemText"] = "打开系统监视器";
    open["isActive"] = true;
    items.push_back(open);

    QMap<QString, QVariant> setting;
    setting["itemId"] = "setting";
    setting["itemText"] = "设置";
    setting["isActive"] = true;
    items.push_back(setting);

    QMap<QString, QVariant> about;
    about["itemId"] = "about";
    about["itemText"] = "关于";
    about["isActive"] = true;
    items.push_back(about);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    // 返回 JSON 格式的菜单数据
    return QJsonDocument::fromVariant(menu).toJson();
}

void SysMonitorPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(checked)

    if (itemKey != pluginName()) {
        return;
    }

    // 根据上面接口设置的 id 执行不同的操作
    if (menuId == "refresh") {
        m_proxyInter->itemUpdate(this, pluginName());
    } else if (menuId == "open") {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.desktopspec.ApplicationManager1")) {
            DDBusSender()
                .service("org.desktopspec.ApplicationManager1")
                .path("/org/desktopspec/ApplicationManager1/deepin_2dsystem_2dmonitor")
                .interface("org.desktopspec.ApplicationManager1.Application")
                .method("Launch")
                .arg(QString())
                .arg(QStringList())
                .arg(QVariantMap())
                .call();
        } else {
            DDBusSender()
                .service("com.deepin.SessionManager")
                .path("/com/deepin/StartManager")
                .interface("com.deepin.StartManager")
                .method("Launch")
                .arg(QString("/usr/share/applications/deepin-system-monitor.desktop"))
                .call();
        }
    } else if (menuId == "setting") {
        int updataInterval = settings.value("updateIntervalSpinBox").toInt();
        m_settingDialog.reset(new pluginSettingDialog(&settings, m_mainWidget));
        connect(m_settingDialog.data(), &pluginSettingDialog::finished, this, [=](int result) {
            if (result == QDialog::Accepted) {
                m_settingDialog->getDisplayContentSetting(&settings);
                writeConfig(&settings);
                if (settings.value("updateIntervalSpinBox").toInt() != updataInterval) {
                    // 修改更新时间间隔
                    m_refreshTimer->start(settings.value("updateIntervalSpinBox").toInt());
                }
            }
        });
        m_settingDialog->show();
    } else if (menuId == "about") {
        m_aboutDialog.reset(new AboutDialog(m_mainWidget));
        m_aboutDialog->show();
    }
}

void SysMonitorPlugin::displayModeChanged(const Dock::DisplayMode displayMode)
{
    dismode = displayMode;
}

void SysMonitorPlugin::positionChanged(const Dock::Position position)
{
    pos = position;
}

PluginsItemInterface::PluginSizePolicy SysMonitorPlugin::pluginSizePolicy() const
{
    return PluginSizePolicy::Custom;
}

// 使用系统配置函数读配置信息
void SysMonitorPlugin::readConfig(Settings *settings)
{
    for (unsigned long i = 0; i < sizeof(settingItems) / sizeof(settingItems[0]); i++) {
        settings->insert(settingItems[i].name, m_proxyInter->getValue(this, settingItems[i].name, settingItems[i].value));
    }
}

// 写配置信息
void SysMonitorPlugin::writeConfig(Settings *settings)
{
    QMapIterator<QString, QVariant> i(*settings);
    while (i.hasNext()) {
        i.next();
        m_proxyInter->saveValue(this, i.key(), i.value());
    }
}

const QString SysMonitorPlugin::toHumanRead(unsigned long l, const char *unit, int digit)
{
    Q_UNUSED(digit)

    int count = 0;
    QString str;
    double f = static_cast<double>(l);
    if (!strcmp(unit, "B"))
        count = 0;
    else if (!strcmp(unit, "KB"))
        count = 1;
    while (f > 999.0) {
        f = f / 1024.0;
        count++;
    }

    if (count == 0) {
        count++;
        f = f / 1024;
    }

    if (f < 0.1)
        str = "  0";
    else if (f < 10.0)
        str = QString::number(f, 'f', 1);
    else if (f < 100.0)
        str = " " + QString::number(f, 'f', 0);
    else
        str = QString::number(f, 'f', 0);

    if (count == 0)
        str += "B";
    else if (count == 1)
        str += "K";
    else if (count == 2)
        str += "M";
    else if (count == 3)
        str += "G";
    else if (count == 4)
        str += "T";
    else if (count == 4)
        str += "P";
    return str;
}

void SysMonitorPlugin::refreshInfo()
{
    // 获得cpu信息
    fp = fopen("/proc/stat", "r");
    if (fp == nullptr) {
        perror("Could not open stat file");
        return;
    }

    user = 0;
    nice = 0;
    system = 0;
    idle = 0;
    iowait = 0;
    irq = 0;
    softirq = 0;
    steal = 0;
    guest = 0;
    guestnice = 0;
    char *ret = fgets(buffer, sizeof(buffer) - 1, fp);
    if (ret == nullptr) {
        perror("Could not read stat file");
        fclose(fp);
        return;
    }

    fclose(fp);
    sscanf(buffer,
           "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu "
           "%16llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest,
           &guestnice);
    worktime = user + nice + system;
    totaltime = user + nice + system + idle + iowait + irq + softirq + steal;
    // 得到百分比
    cpuPercent =
        qRound((worktime - oldworktime) * 100.0 / (totaltime - oldtotaltime));
    oldtotaltime = totaltime;
    oldworktime = worktime;
    info.cpu = cpuPercent;
    info.scpu.clear();
    if (cpuPercent <= 9) {
        info.scpu = " ";
    }
    info.scpu += QString::number(cpuPercent) + "%";

    // 获得内存信息
    fp = fopen("/proc/meminfo", "r");
    if (fp == nullptr) {
        perror("Could not open meminfo file");
        return;
    }

    do {
        ret = fgets(buffer, sizeof(buffer) - 1, fp);
        if (ret == nullptr) {
            perror("Could not read meminfo file");
            fclose(fp);
            return;
        }
        sscanf(buffer, "%s %lu kB", devname, &tmp);
        if (!strcmp(devname, "MemTotal:"))
            totalmem = tmp;
        else if (!strcmp(devname, "MemAvailable:"))
            availablemem = tmp;
        else if (!strcmp(devname, "SwapTotal:"))
            totalswap = tmp;
        else if (!strcmp(devname, "SwapFree:"))
            freeswap = tmp;
    } while (strcmp(devname, "SwapFree:"));
    fclose(fp);
    memPercent = qRound((totalmem - availablemem) * 100.0 / totalmem);
    info.mem = memPercent;
    info.smem.clear();
    if (memPercent <= 9) {
        info.smem = " ";
    }
    info.smem += QString::number(memPercent) + "%";

    swapPercent = qRound((totalswap - freeswap) * 100.0 / totalswap);
    strswap.clear();
    if (swapPercent <= 9) {
        strswap = " ";
    } else {
        strswap += QString::number(swapPercent) + "%";
    }

    // 获得net信息
    fp = fopen("/proc/net/dev", "r");
    if (fp == nullptr) {
        perror("Could not open netdev file");
        return;
    }

    ret = fgets(buffer, sizeof(buffer) - 1, fp);
    ret = fgets(buffer, sizeof(buffer) - 1, fp);
    if (ret == nullptr) {
        perror("Could not read netdev file");
        fclose(fp);
        return;
    }

    tmpr = tmps = rbytes = sbytes = 0;
    while (true) {
        ret = fgets(buffer, sizeof(buffer) - 1, fp);
        if (ret == nullptr)
            break;
        sscanf(buffer, "%s %lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %lu", devname,
               &tmpr, &tmps);
        if (strcmp(devname, "lo:") == 0)
            continue;
        rbytes += tmpr;
        sbytes += tmps;
    }
    fclose(fp);
    // 考虑到读取间隔不一定是1s，要运算成1s的量
    tmpr = (oldrbytes == 0 ? 0 : rbytes - oldrbytes) * 1000 / settings.value("updateIntervalSpinBox").toUInt();
    tmps = (oldsbytes == 0 ? 0 : sbytes - oldsbytes) * 1000 / settings.value("updateIntervalSpinBox").toUInt();
    oldrbytes = rbytes;
    oldsbytes = sbytes;

    info.netup = static_cast<int>(tmps);
    info.netdown = static_cast<int>(tmpr);
    info.snetup = toHumanRead(tmps, "B", 0);
    info.snetdwon = toHumanRead(tmpr, "B", 0);

    // 每10s执行一次，降低cpu开销
    if (settings.value("batInfoComboBox").toInt() == 1 && bat_count == 0) {
        // 使用upower命令获得电池信息，兼容性最好，deepin默认预装有upower
        fp = nullptr;
        // 使用popen执行shell命令并返回一个流来读取电池信息
        fp = popen("upower -i $(upower -e | grep 'BAT') | grep -E 'energy-rate'",
                   "r");
        if (fp == nullptr) {
            perror("popen");
            return;
        }

        battery_watts = -1.0;
        fscanf(fp, "    energy-rate:         %lf W", &battery_watts);
        pclose(fp);

        // 使用sensors获得CPU温度
        fp = nullptr;
        fp = popen("sensors | grep 'Core 0'", "r");
        info.cputemp = 0;
        if (fp != nullptr) {
            fscanf(fp, "Core 0:        %lf°C", &info.cputemp);
        } else {
            perror("Could not run sensors command");
        }
        pclose(fp);
        info.scputemp = QString("%1℃").arg(info.cputemp);
    }
    // 大于等于10秒就归零
    bat_count *settings.value("updateIntervalSpinBox").toInt() >= 10 * 1000
        ? bat_count = 0
        : bat_count++;

    // 更新内容
    m_mainWidget->updateData(info, pos, settings);
    /**
     * NOTE: plugin is setFixedSize when loaded by
     * dde-tray-loader, which cause incorrect size
     * so remove constraints manually here
     */
    m_mainWidget->setMinimumSize(0, 0);
    m_mainWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    if (m_tipsWidget->isVisible()) {
        updateWidget(m_tipsWidget);
    }

    m_proxyInter->itemUpdate(this, pluginName());
    // qDebug()<<"m_mainWidget->height():"<<m_mainWidget->height();
    // qDebug()<<"m_mainWidget->width():"<<m_mainWidget->width();
    // qDebug()<<"m_pluginWidget->height():"<<m_pluginWidget->height();
    // m_pluginWidget->m_infoLabel->setMinimumHeight(29);
    // qDebug()<<"m_appletWidget->isVisible():"<<QString::number(m_appletWidget->isVisible());
    // qDebug()<<"m_tipsWidget->isVisible():"<<QString::number(m_tipsWidget->isVisible());
}

void SysMonitorPlugin::updateWidget(Dock::TipsWidget *widget)
{
    // 设置/刷新 tips 中的信息
    QString baseInfo = QString("CPU:  %1 %2\n"
                               "MEM: %3/%4=%5\n"
                               "SWAP:%6/%7=%8\n"
                               "UP:  %9 %10/s\n"
                               "DOWN:%11 %12/s")
                           .arg(info.scpu)
                           .arg(info.scputemp)
                           .arg(toHumanRead(totalmem - availablemem, "KB", 1))
                           .arg(toHumanRead(totalmem, "KB", 1))
                           .arg(info.smem)
                           .arg(toHumanRead(totalswap - freeswap, "KB", 1))
                           .arg(toHumanRead(totalswap, "KB", 1))
                           .arg(strswap)
                           .arg(toHumanRead(oldsbytes, "B", 1))
                           .arg(toHumanRead(tmps, "B", 1))
                           .arg(toHumanRead(oldrbytes, "B", 1))
                           .arg(toHumanRead(tmpr, "B", 1));

    QString batInfo("");
    if (battery_watts >= 0 && settings.value("batInfoComboBox").toInt()) {
        batInfo = QString("\nBAT: %1W").arg(QString::number(battery_watts, 'f', 2));
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    widget->setTextList(QString(baseInfo + batInfo).split("\n", Qt::SkipEmptyParts));
#else
    widget->setTextList(QString(baseInfo + batInfo).split("\n", QString::SkipEmptyParts));
#endif
}
