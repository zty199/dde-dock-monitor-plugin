#ifndef SYSMONITORPLUGIN_H
#define SYSMONITORPLUGIN_H

#include "type.h"

#include <pluginsiteminterface.h>

#include <QLabel>
#include <QObject>
#include <QTimer>
#include <QScopedPointer>

namespace Dock {
class TipsWidget;
}

class MainWidget;
class pluginSettingDialog;
class AboutDialog;
class WidgetPlugin;
class SysMonitorPlugin : public QObject
    , PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID ModuleInterface_iid FILE "sys-monitor.json")

public:
    explicit SysMonitorPlugin(QObject *parent = nullptr);
    ~SysMonitorPlugin() override;

    const QString pluginDisplayName() const override;
    const QString pluginName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;

    bool pluginIsAllowDisable() override;
    bool pluginIsDisable() override;
    void pluginStateSwitched() override;

    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;
    void displayModeChanged(const Dock::DisplayMode displayMode) override;
    void positionChanged(const Dock::Position position) override;

    PluginSizePolicy pluginSizePolicy() const override;

private:
    // 自定义读写配置函数
    void readConfig(Settings *settings);
    void writeConfig(Settings *settings);

    const QString toHumanRead(unsigned long l, const char *unit, int digit);

private slots:
    // 用于更新数据的槽函数
    void refreshInfo();
    // 更新tipsWidget气泡数据的函数
    void updateWidget(Dock::TipsWidget *widget);

public:
    static struct SettingItem settingItems[]; // 公共的保存默认设置的数组

private:
    // CPU工作时间除以总时间,内存百分比,交换区百分比
    int cpuPercent, memPercent, swapPercent;
    QString strswap;
    // 获取cpu总时间,获取cpu工作时间
    unsigned long long totaltime, worktime;
    // 保存上一次结果
    unsigned long long oldtotaltime, oldworktime;
    // cpu的各种时间变量
    unsigned long long user, nice, system, idle;
    unsigned long long iowait, irq, softirq, steal, guest, guestnice;
    // 字符数组保存文件内容
    char buffer[1024];
    // 文件描述符
    FILE *fp;
    // 总内存，可用内存
    unsigned long totalmem, availablemem, tmp, totalswap, freeswap;
    // 接收字节数，发送字节数
    unsigned long rbytes, sbytes, oldrbytes, oldsbytes, tmpr, tmps;
    char devname[1024];
    // 电池功率瓦特
    double battery_watts;
    // 电池统计计数，每隔这么多次才读取一次
    int bat_count;
    // 字体
    QFont font;
    // dock显示模式:时尚模式 高效模式
    Dock::DisplayMode dismode;
    // dock的位置：上下左右
    Dock::Position pos;
    // 设置结构体
    Settings settings;
    // 传递给widget的信息结构体
    Info info;

private:
    // 处理时间间隔的计时器
    QTimer *m_refreshTimer = nullptr;

    MainWidget *m_mainWidget = nullptr;
    Dock::TipsWidget *m_tipsWidget = nullptr;

    QScopedPointer<pluginSettingDialog> m_settingDialog;
    QScopedPointer<AboutDialog> m_aboutDialog;

    WidgetPlugin *m_proxyInter = nullptr;
};

#endif // SYSMONITORPLUGIN_H
