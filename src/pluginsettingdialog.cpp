#include "pluginsettingdialog.h"
#include "ui_pluginsettingdialog.h"

#include <QColorDialog>
#include <QDebug>

pluginSettingDialog::pluginSettingDialog(Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::pluginSettingDialog)
{
    ui->setupUi(this);

    if (settings->value("chartModeCheckBox").toInt()) {
        ui->tabWidget->setCurrentIndex(1);
    } else {
        ui->tabWidget->setCurrentIndex(0);
    }
    pal = QPalette(); // 首先初始化画板

    QMapIterator<QString, QVariant> i(*settings);
    while (i.hasNext()) {
        i.next();
        // 找到以key为名字的widget
        QWidget *obj = findChild<QWidget *>(i.key());
        if (obj == nullptr) {
            qDebug() << "不能找到对象名为：" << i.key();
            continue;
        }
        if (obj->metaObject()->className() == QStringLiteral("QComboBox")) {
            QComboBox *cb = qobject_cast<QComboBox *>(obj);
            cb->setCurrentIndex(i.value().toInt());
        } else if (obj->metaObject()->className() == QStringLiteral("QCheckBox")) {
            QCheckBox *cb = qobject_cast<QCheckBox *>(obj);
            cb->setChecked(i.value().toInt());
        } else if (obj->metaObject()->className() == QStringLiteral("QSpinBox")) {
            QSpinBox *sb = qobject_cast<QSpinBox *>(obj);
            sb->setValue(i.value().toInt());
        } else if (obj->metaObject()->className() == QStringLiteral("QWidget")) {
            QWidget *wg = qobject_cast<QWidget *>(obj);
            pal.setColor(QPalette::Window, i.value().value<QColor>());
            wg->setAutoFillBackground(true);
            wg->setPalette(pal);
            // qDebug()<<"颜色是："<<i.value().value<QColor>();
        } else if (obj->metaObject()->className() == QStringLiteral("QLineEdit")) {
            QLineEdit *le = qobject_cast<QLineEdit *>(obj);
            le->setText(i.value().value<QString>());
        }
    }
    foreach (QPushButton *btn, findChildren<QPushButton *>(QRegularExpression("\\w*ColorPushButton"))) {
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(selectColor(void)));
    }
}

pluginSettingDialog::~pluginSettingDialog()
{
    delete ui;
}

void pluginSettingDialog::getDisplayContentSetting(Settings *settings)
{
    QMapIterator<QString, QVariant> i(*settings);
    while (i.hasNext()) {
        i.next();
        // 找到以key为名字的widget
        QWidget *obj = findChild<QWidget *>(i.key());
        if (obj == nullptr) {
            qDebug() << "不能找到对象名为：" << i.key();
            continue;
        }
        if (obj->metaObject()->className() == QStringLiteral("QComboBox")) {
            QComboBox *cb = qobject_cast<QComboBox *>(obj);
            settings->insert(cb->objectName(), cb->currentIndex());
        } else if (obj->metaObject()->className() == QStringLiteral("QCheckBox")) {
            QCheckBox *cb = qobject_cast<QCheckBox *>(obj);
            settings->insert(cb->objectName(), cb->isChecked());
        } else if (obj->metaObject()->className() == QStringLiteral("QSpinBox")) {
            QSpinBox *sb = qobject_cast<QSpinBox *>(obj);
            settings->insert(sb->objectName(), sb->value());
        } else if (obj->metaObject()->className() == QStringLiteral("QWidget")) {
            QWidget *wg = qobject_cast<QWidget *>(obj);
            settings->insert(wg->objectName(), wg->palette().window().color());
        } else if (obj->metaObject()->className() == QStringLiteral("QLineEdit")) {
            QLineEdit *le = qobject_cast<QLineEdit *>(obj);
            settings->insert(le->objectName(), le->text());
        }
    }
}

void pluginSettingDialog::selectColor()
{
    QWidget *colorWidget;
    //"netUpColorPushButton"-->"netUpWidget"
    colorWidget = findChild<QWidget *>(sender()->objectName().replace("ColorPushButton", "Widget"));

    QColor color = QColorDialog::getColor(colorWidget->palette().window().color(),
                                          this, tr("颜色对话框"), QColorDialog::ShowAlphaChannel);
    if (color != QColor::Invalid) {
        pal.setColor(QPalette::Window, color);
        colorWidget->setAutoFillBackground(true);
        colorWidget->setPalette(pal);
    }
}
