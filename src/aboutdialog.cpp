#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_blogPushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://blog.mxslly.com"));
}

void AboutDialog::on_giteePushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://gitee.com/q77190858/dde-sys-monitor-plugin"));
}

void AboutDialog::on_githubPushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/q77190858/dde-sys-monitor-plugin"));
}
