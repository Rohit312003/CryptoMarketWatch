#include "configmanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

ConfigManager& ConfigManager::instance()
{
    static ConfigManager s_instance;
    return s_instance;
}

ConfigManager::ConfigManager()

{
    // Use current application directory
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/config.ini";
    m_configFile = configFile;
    m_settings = new QSettings(m_configFile, QSettings::IniFormat);

    qDebug() << "Config file location:" << configFile;

    // Create QSettings pointing to the config file in app directory
    m_settings = new QSettings(configFile, QSettings::IniFormat);

    loadConfig();
}

ConfigManager::~ConfigManager()
{
    if (m_settings) {
        m_settings->sync();  // Save before deletion
        delete m_settings;
    }
}
QString ConfigManager::configPath() const {
    return m_configFile;
}


void ConfigManager::loadConfig()
{
    // Load or set default username/password
    m_username = m_settings->value("Credentials/username", "").toString();
    m_password = m_settings->value("Credentials/password", "").toString();

    // Load favorite pairs (default 20+ popular cryptos)
    if (m_settings->contains("Pairs/favorites")) {
        m_favoritePairs = m_settings->value("Pairs/favorites").toStringList();
    } else {
        // Default list of 20+ popular crypto pairs
        m_favoritePairs = {
            "BTC-USDT", "ETH-USDT", "XRP-USDT", "ADA-USDT", "SOL-USDT",
            "DOGE-USDT", "MATIC-USDT", "AVAX-USDT", "LINK-USDT", "ATOM-USDT",
            "DOT-USDT", "LTC-USDT", "NEAR-USDT", "ARB-USDT", "OP-USDT",
            "LIDO-USDT", "STETH-USDT", "APE-USDT", "UNISWAP-USDT", "AAVE-USDT",
            "FIL-USDT", "FTM-USDT"
        };
        m_settings->setValue("Pairs/favorites", m_favoritePairs);
        m_settings->sync();  // Save defaults immediately
    }


}

void ConfigManager::setUsername(const QString& username)
{
    m_username = username;
    m_settings->setValue("Credentials/username", username);
    m_settings->sync();
}

QString ConfigManager::getUsername() const
{
    return m_username;
}

void ConfigManager::setPassword(const QString& password)
{
    m_password = password;
    m_settings->setValue("Credentials/password", password);
    m_settings->sync();
}

QString ConfigManager::getPassword() const
{
    return m_password;
}

void ConfigManager::setFavoritePairs(const QStringList& pairs)
{
    m_favoritePairs = pairs;
    m_settings->setValue("Pairs/favorites", pairs);
    m_settings->sync();
}

QStringList ConfigManager::getFavoritePairs() const
{
    return m_favoritePairs;
}

void ConfigManager::addFavoritePair(const QString& pair)
{
    if (!m_favoritePairs.contains(pair)) {
        m_favoritePairs.append(pair);
        m_settings->setValue("Pairs/favorites", m_favoritePairs);
        m_settings->sync();
    }
}

void ConfigManager::removeFavoritePair(const QString& pair)
{
    if (m_favoritePairs.removeAll(pair) > 0) {
        m_settings->setValue("Pairs/favorites", m_favoritePairs);
        m_settings->sync();
    }
}



void ConfigManager::save()
{
    m_settings->sync();
}
