#include "mainwidget.h"

#include <DGuiApplicationHelper>

#include <QScreen>
#include <QtMath>

DGUI_USE_NAMESPACE

MainWidget::MainWidget(Settings &settings, Dock::Position position)
{
    centralLayout = nullptr;
    cpuMemLabel = nullptr;
    netLabel = nullptr;
    netChart = nullptr;
    cpuChart = nullptr;
    memChart = nullptr;
    // setMinimumSize(5,5);
    //设置等宽字体
    font.setFamily("Noto Mono");
    // 获取dpi，一般默认都是96，根据dpi进行字体的缩放，直接设置pointsize无法解决hidpi问题
    dpi = qIntCast(QGuiApplication::primaryScreen()->logicalDotsPerInch());
    oldsettings = settings;
    oldposition = position;

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=]() {
        updateData(oldinfo, oldposition, oldsettings);
    });
}

MainWidget::~MainWidget()
{
    if (cpuMemLabel != nullptr) {
        delete cpuMemLabel;
        cpuMemLabel = nullptr;
    }
    if (netLabel != nullptr) {
        delete netLabel;
        netLabel = nullptr;
    }
    if (netChart != nullptr) {
        delete netChart;
        netChart = nullptr;
    }
    if (cpuChart != nullptr) {
        delete cpuChart;
        cpuChart = nullptr;
    }
    if (memChart != nullptr) {
        delete memChart;
        memChart = nullptr;
    }
    if (centralLayout != nullptr) {
        delete centralLayout;
        centralLayout = nullptr;
    }
}

void MainWidget::updateData(const Info &info, Dock::Position position, const Settings &settings)
{
    oldinfo = info;
    // qDebug()<<"MainWidget::UpdateData() start";
    //如果dock位置发生如下变化，则手动重构ui
    if (((oldposition == Dock::Top || oldposition == Dock::Bottom) && (position == Dock::Left || position == Dock::Right))
        || ((position == Dock::Top || position == Dock::Bottom) && (oldposition == Dock::Left || oldposition == Dock::Right))) {
        if (cpuMemLabel != nullptr) {
            delete cpuMemLabel;
            cpuMemLabel = nullptr;
        }
        if (netLabel != nullptr) {
            delete netLabel;
            netLabel = nullptr;
        }
        if (netChart != nullptr) {
            delete netChart;
            netChart = nullptr;
        }
        if (cpuChart != nullptr) {
            delete cpuChart;
            cpuChart = nullptr;
        }
        if (memChart != nullptr) {
            delete memChart;
            memChart = nullptr;
        }
        if (centralLayout != nullptr) {
            delete centralLayout;
            centralLayout = nullptr;
        }
    }
    oldposition = position;

    if (!settings.value("chartModeCheckBox").toInt()) //文字模式
    {
        //当模式切换的时候
        if (oldsettings.value("chartModeCheckBox").toInt()) {
            //先清理掉之前的ui
            // qDebug()<<"模式切换，先清理掉之前的图表ui";
            if (netChart != nullptr) {
                delete netChart;
                netChart = nullptr;
            }
            if (cpuChart != nullptr) {
                delete cpuChart;
                cpuChart = nullptr;
            }
            if (memChart != nullptr) {
                delete memChart;
                memChart = nullptr;
            }
            if (centralLayout != nullptr) {
                delete centralLayout;
                centralLayout = nullptr;
            }
        }
        //当模式切换或者第一次初始化的时候，需要新建ui
        if (centralLayout == nullptr) {
            //创建新的ui
            qDebug() << "创建新的文字ui";
            cpuMemLabel = new QLabel();
            cpuMemLabel->setAlignment(Qt::AlignCenter);
            netLabel = new QLabel();
            netLabel->setAlignment(Qt::AlignCenter);
            centralLayout = new QBoxLayout((position == Dock::Top || position == Dock::Bottom) ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
            centralLayout->addWidget(cpuMemLabel);
            centralLayout->addWidget(netLabel);
            centralLayout->setMargin(0);
            setLayout(centralLayout);
        }
        centralLayout->setSpacing(settings.value("wordSpacingSpinBox").toInt());
        font.setPixelSize((dpi * settings.value("fontSizeSpinBox").toInt()) / 72);
        cpuMemLabel->setFont(font);
        netLabel->setFont(font);

        QString style;
        switch (settings.value("fontColorComboBox").toInt()) {
        case 0:
            style = QString("QLabel {color: %1;}").arg("#fff");
            break;
        case 1:
            style = QString("QLabel {color: %1;}").arg("#000");
            break;
        case 2:
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
                style = QString("QLabel {color: %1;}").arg("#000");
            else
                style = QString("QLabel {color: %1;}").arg("#fff");
            break;
        }

        cpuMemLabel->setStyleSheet(style);
        netLabel->setStyleSheet(style);

        switch (settings.value("displayContentComboBox").toInt()) {
        case DisplayContentSetting::CPUMEM:
            cpuMemLabel->setVisible(true);
            netLabel->setVisible(false);
            cpuMemLabel->setText(QString("%1%2\n%3%4")
                                     .arg(settings.value("cpuDiyWordLineEdit").value<QString>())
                                     .arg(info.scpu)
                                     .arg(settings.value("memDiyWordLineEdit").value<QString>())
                                     .arg(info.smem));
            break;
        case DisplayContentSetting::NETSPEED:
            cpuMemLabel->setVisible(false);
            netLabel->setVisible(true);
            netLabel->setText(QString("%1%2/s\n%3%4/s")
                                  .arg(settings.value("upNetspeedDiyWordLineEdit").value<QString>())
                                  .arg(info.snetup)
                                  .arg(settings.value("downNetspeedDiyWordLineEdit").value<QString>())
                                  .arg(info.snetdwon));
            break;
        default: // DisplayContentSetting::ALL:
            cpuMemLabel->setVisible(true);
            netLabel->setVisible(true);
            cpuMemLabel->setText(QString("%1%2\n%3%4")
                                     .arg(settings.value("cpuDiyWordLineEdit").value<QString>())
                                     .arg(info.scpu)
                                     .arg(settings.value("memDiyWordLineEdit").value<QString>())
                                     .arg(info.smem));
            netLabel->setText(QString("%1%2/s\n%3%4/s")
                                  .arg(settings.value("upNetspeedDiyWordLineEdit").value<QString>())
                                  .arg(info.snetup)
                                  .arg(settings.value("downNetspeedDiyWordLineEdit").value<QString>())
                                  .arg(info.snetdwon));
            break;
        }
    } else //图表模式
    {
        //当模式切换的时候，清除文字ui
        if (!oldsettings.value("chartModeCheckBox").toInt()) {
            delete cpuMemLabel;
            cpuMemLabel = nullptr;
            delete netLabel;
            netLabel = nullptr;
            delete centralLayout;
            centralLayout = nullptr;
        }
        //当图表有变化的时候，清除所有图表ui
        if (oldsettings.value("cpuChartCheckBox").toInt() + settings.value("cpuChartCheckBox").toInt() == 1 || oldsettings.value("memChartCheckBox").toInt() + settings.value("memChartCheckBox").toInt() == 1 || oldsettings.value("netChartCheckBox").toInt() + settings.value("netChartCheckBox").toInt() == 1) {
            if (cpuChart != nullptr) {
                delete cpuChart;
                cpuChart = nullptr;
            }
            if (memChart != nullptr) {
                delete memChart;
                memChart = nullptr;
            }
            if (netChart != nullptr) {
                delete netChart;
                netChart = nullptr;
            }
            delete centralLayout;
            centralLayout = nullptr;
        }
        //当模式切换或者第一次初始化的时候，绘制图表ui
        if (centralLayout == nullptr) {
            centralLayout = new QBoxLayout((position == Dock::Top || position == Dock::Bottom) ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
            centralLayout->setMargin(0);
            setLayout(centralLayout);
        }

        centralLayout->setSpacing(settings.value("chartSpacingSpinBox").toInt());
        if (settings.value("cpuChartCheckBox").toInt()) {
            if (cpuChart == nullptr) {
                cpuChart = new StreamChart();
                centralLayout->addWidget(cpuChart);
            }
            data.x = info.cpu;
            data.y = 0;
            cpuChart->height = settings.value("cpuHeightSpinBox").toInt();
            cpuChart->width = settings.value("cpuWidthSpinBox").toInt();
            cpuChart->color1 = settings.value("cpuWorkWidget").value<QColor>();
            cpuChart->color2 = QColor(255, 255, 255, 0);
            cpuChart->borderRound = settings.value("cpuBorderRoundSpinBox").toInt();
            cpuChart->colorBorder = settings.value("cpuBorderWidget").value<QColor>();
            cpuChart->colorBackground = settings.value("cpuBackgroundWidget").value<QColor>();
            cpuChart->spacing = settings.value("chartSpacingSpinBox").toInt();

            cpuChart->updateChart(data);
        }
        if (settings.value("memChartCheckBox").toInt()) {
            if (memChart == nullptr) {
                memChart = new StreamChart();
                centralLayout->addWidget(memChart);
            }
            data.x = info.mem;
            data.y = 0;
            memChart->height = settings.value("memHeightSpinBox").toInt();
            memChart->width = settings.value("memWidthSpinBox").toInt();
            memChart->color1 = settings.value("memUsedWidget").value<QColor>();
            memChart->color2 = QColor(255, 255, 255, 0);
            memChart->borderRound = settings.value("memBorderRoundSpinBox").toInt();
            memChart->colorBorder = settings.value("memBorderWidget").value<QColor>();
            memChart->colorBackground = settings.value("memBackgroundWidget").value<QColor>();
            memChart->spacing = settings.value("chartSpacingSpinBox").toInt();

            memChart->updateChart(data);
        }
        if (settings.value("netChartCheckBox").toInt()) {
            if (netChart == nullptr) {
                netChart = new StreamChart();
                centralLayout->addWidget(netChart);
            }
            data.x = info.netdown * 100 / 1024 / settings.value("netDownTopSpinBox").toInt();
            data.y = info.netup * 100 / 1024 / settings.value("netUpTopSpinBox").toInt();
            netChart->height = settings.value("netHeightSpinBox").toInt();
            netChart->width = settings.value("netWidthSpinBox").toInt();
            netChart->color1 = settings.value("netDownWidget").value<QColor>();
            netChart->color2 = settings.value("netUpWidget").value<QColor>();
            netChart->borderRound = settings.value("netBorderRoundSpinBox").toInt();
            netChart->colorBorder = settings.value("netBorderWidget").value<QColor>();
            netChart->colorBackground = settings.value("netBackgroundWidget").value<QColor>();
            netChart->spacing = settings.value("chartSpacingSpinBox").toInt();

            netChart->updateChart(data);
        }
    }
    oldsettings = settings;
    // qDebug()<<"MainWidget::UpdateData() finished";
}

QSize MainWidget::sizeHint() const
{
    if (centralLayout == nullptr)
        return QSize(100, 30);
    int w, h;
    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
    if (!oldsettings.value("chartModeCheckBox").toInt()) //文字模式
    {
        switch (oldsettings.value("displayContentComboBox").toInt()) {
        case DisplayContentSetting::CPUMEM:
            w = QFontMetrics(font).boundingRect(cpuMemLabel->text()).size().width() / 2;
            h = QFontMetrics(font).boundingRect(cpuMemLabel->text()).size().height() * 2;
            break;
        case DisplayContentSetting::NETSPEED:
            w = QFontMetrics(font).boundingRect(netLabel->text()).size().width() / 2;
            h = QFontMetrics(font).boundingRect(netLabel->text()).size().height() * 2;
            break;
        case DisplayContentSetting::ALL:
            if (position == Dock::Top || position == Dock::Bottom) {
                w = QFontMetrics(font).boundingRect(cpuMemLabel->text()).size().width() / 2 + QFontMetrics(font).boundingRect(netLabel->text()).size().width() / 2;
                h = qMax(QFontMetrics(font).boundingRect(cpuMemLabel->text()).size().height() * 2,
                         QFontMetrics(font).boundingRect(netLabel->text()).size().height() * 2);
            } else {
                w = qMax(QFontMetrics(font).boundingRect(cpuMemLabel->text()).size().width() / 2,
                         QFontMetrics(font).boundingRect(netLabel->text()).size().width() / 2);
                h = QFontMetrics(font).boundingRect(cpuMemLabel->text()).size().height() * 2 + QFontMetrics(font).boundingRect(netLabel->text()).size().height() * 2;
            }
            break;
        default:
            w = QFontMetrics(font).boundingRect(netLabel->text()).size().width() / 2;
            h = QFontMetrics(font).boundingRect(netLabel->text()).size().height() * 2;
            break;
        }
    } else //图表模式
    {
        if (position == Dock::Top || position == Dock::Bottom) {
            w = (cpuChart == nullptr ? 0 : cpuChart->width) + (memChart == nullptr ? 0 : memChart->width) + (netChart == nullptr ? 0 : netChart->width);
            h = qMax(cpuChart == nullptr ? 0 : cpuChart->height,
                     qMax(memChart == nullptr ? 0 : memChart->height, netChart == nullptr ? 0 : netChart->height));
        } else {
            w = qMax(cpuChart == nullptr ? 0 : cpuChart->width,
                     qMax(memChart == nullptr ? 0 : memChart->width, netChart == nullptr ? 0 : netChart->width));
            h = (cpuChart == nullptr ? 0 : cpuChart->height) + (memChart == nullptr ? 0 : memChart->height) + (netChart == nullptr ? 0 : netChart->height);
        }
    }
    QSize size(w, h);
    // qDebug()<<size;
    // qDebug()<<QString("width:%1    height:%2").arg(width()).arg(height());
    return size;
}
