#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "streamchart.h"
#include "type.h"

#include <pluginsiteminterface.h>

#include <QWidget>
#include <QLayout>
#include <QLabel>

extern struct SettingItem settingItems[];

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(Settings &, Dock::Position);
    ~MainWidget();
    QSize sizeHint() const;

    void updateData(const Info &info, Dock::Position position, const Settings &settings);

public:
    int dpi;
    QBoxLayout *centralLayout;
    // 文字模式数据显示在这2个Label上
    QLabel *cpuMemLabel, *netLabel;
    //显示数据的图表类
    StreamChart *netChart, *cpuChart, *memChart;
    struct Data data;
    // 字体
    QFont font;
    //保存之前的信息
    Info oldinfo;
    //保存之前的设置
    Settings oldsettings;
    //保存之前的位置
    Dock::Position oldposition;

private:
    Ui::MainWidget *ui;
};

#endif // MAINWIDGET_H
