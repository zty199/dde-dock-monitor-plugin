#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "streamchart.h"
#include "type.h"

#include <pluginsiteminterface.h>

#include <QWidget>
#include <QLayout>
#include <QLabel>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(Settings &, Dock::Position, QWidget *parent = nullptr);
    ~MainWidget() override;
    QSize sizeHint() const override;

public slots:
    void updateData(const Info &info, Dock::Position position, const Settings &settings);

public:
    QBoxLayout *centralLayout;
    // 文字模式数据显示在这2个Label上
    QLabel *cpuMemLabel, *netLabel;
    // 显示数据的图表类
    StreamChart *netChart, *cpuChart, *memChart;

    struct Data data;
    int dpi;
    // 字体
    QFont font;
    // 保存之前的信息
    Info oldinfo;
    // 保存之前的设置
    Settings oldsettings;
    // 保存之前的位置
    Dock::Position oldposition;
};

#endif // MAINWIDGET_H
