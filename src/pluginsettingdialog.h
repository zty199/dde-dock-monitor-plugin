#ifndef PLUGINSETTINGDIALOG_H
#define PLUGINSETTINGDIALOG_H

#include "type.h"

#include <QDialog>

namespace Ui {
class pluginSettingDialog;
}

class pluginSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit pluginSettingDialog(Settings *settings, QWidget *parent = nullptr);
    ~pluginSettingDialog();

    //公有函数获得显示设置
    void getDisplayContentSetting(Settings *settings);

private slots:
    void selectColor(void);

private:
    Ui::pluginSettingDialog *ui;

    //公用的画板
    QPalette pal;
};

#endif // PLUGINSETTINGDIALOG_H
