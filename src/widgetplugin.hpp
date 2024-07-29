#ifndef WIDGETPLUGIN_HPP
#define WIDGETPLUGIN_HPP

#include <pluginproxyinterface.h>
#include <pluginsiteminterface.h>

#include <QGSettings>

#include <QObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

constexpr char kGSettingsSchemaId[] = "com.deepin.dde.dock";
constexpr char kGSettingsPath[] = "";
const QString kGSettingsKey = "pluginSettings";

class WidgetPlugin : public QObject
    , PluginProxyInterface
{
    Q_OBJECT

public:
    explicit WidgetPlugin(PluginProxyInterface *proxyInter, QObject *parent = nullptr)
        : QObject(parent)
        , m_proxyInter(proxyInter)
    {
        if (QGSettings::isSchemaInstalled(kGSettingsSchemaId)) {
            m_gsettings = new QGSettings(kGSettingsSchemaId, kGSettingsPath, this);
        }
    }

    void itemAdded(PluginsItemInterface *const itemInter, const QString &itemKey) override
    {
        m_proxyInter->itemAdded(itemInter, itemKey);
    }

    void itemUpdate(PluginsItemInterface *const itemInter, const QString &itemKey) override
    {
        m_proxyInter->itemUpdate(itemInter, itemKey);
    }

    void itemRemoved(PluginsItemInterface *const itemInter, const QString &itemKey) override
    {
        m_proxyInter->itemRemoved(itemInter, itemKey);
    }

    void requestWindowAutoHide(PluginsItemInterface *const itemInter, const QString &itemKey, const bool autoHide) override
    {
        m_proxyInter->requestWindowAutoHide(itemInter, itemKey, autoHide);
    }

    void requestRefreshWindowVisible(PluginsItemInterface *const itemInter, const QString &itemKey) override
    {
        m_proxyInter->requestRefreshWindowVisible(itemInter, itemKey);
    }

    void requestSetAppletVisible(PluginsItemInterface *const itemInter, const QString &itemKey, const bool visible) override
    {
        m_proxyInter->requestSetAppletVisible(itemInter, itemKey, visible);
    }

    void saveValue(PluginsItemInterface *const itemInter, const QString &key, const QVariant &value) override
    {
        if (m_gsettings == nullptr) {
            return m_proxyInter->saveValue(itemInter, key, value);
        }

        QVariant oldValue = m_gsettings->get(kGSettingsKey);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(oldValue.toString().toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            return m_proxyInter->saveValue(itemInter, key, value);
        }

        QJsonObject obj = doc.object();
        QJsonObject data = obj.value(itemInter->pluginName()).toObject();
        data.insert(key, QJsonValue::fromVariant(value));
        obj.insert(itemInter->pluginName(), data);
        m_gsettings->set(kGSettingsKey, QJsonDocument(obj).toJson(QJsonDocument::Compact));
    }

    const QVariant getValue(PluginsItemInterface *const itemInter, const QString &key, const QVariant &fallback) override
    {
        if (m_gsettings == nullptr) {
            return m_proxyInter->getValue(itemInter, key, fallback);
        }

        QVariant value = m_gsettings->get(kGSettingsKey);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(value.toString().toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            return m_proxyInter->getValue(itemInter, key, fallback);
        }

        QJsonObject obj = doc.object();
        QJsonObject data = obj.value(itemInter->pluginName()).toObject();
        return data.contains(key) ? data.value(key).toVariant() : fallback;
    }

    void removeValue(PluginsItemInterface *const itemInter, const QStringList &keyList) override
    {
        if (m_gsettings == nullptr) {
            return m_proxyInter->removeValue(itemInter, keyList);
        }

        QVariant value = m_gsettings->get(kGSettingsKey);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(value.toString().toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            return m_proxyInter->removeValue(itemInter, keyList);
        }

        QJsonObject obj = doc.object();
        QJsonObject data = obj.value(itemInter->pluginName()).toObject();
        foreach (const QString &key, keyList) {
            data.remove(key);
        }
        data.isEmpty() ? obj.remove(itemInter->pluginName())
                       : (void)obj.insert(itemInter->pluginName(), data);
        m_gsettings->set(kGSettingsKey, QJsonDocument(obj).toJson(QJsonDocument::Compact));
    }

private:
    PluginProxyInterface *m_proxyInter = nullptr;

    QGSettings *m_gsettings = nullptr;
};

#endif // WIDGETPLUGIN_HPP
