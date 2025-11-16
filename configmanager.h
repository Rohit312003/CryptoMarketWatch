#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QStringList>
#include <QSettings>

class ConfigManager
{
public:
    static ConfigManager& instance();

    // Credentials
    void setUsername(const QString& username);
    QString getUsername() const;

    void setPassword(const QString& password);
    QString getPassword() const;


    QString configPath() const;

    // Crypto pairs (favorites)
    void setFavoritePairs(const QStringList& pairs);
    QStringList getFavoritePairs() const;
    void addFavoritePair(const QString& pair);
    void removeFavoritePair(const QString& pair);



    // Save and load (automatic, but can call manually)
    void save();

private:
    ConfigManager();
    ~ConfigManager();

    QSettings* m_settings;
    QString m_username;
    QString m_password;
    QString m_configFile;
    QStringList m_favoritePairs;


    void loadConfig();
};

#endif // CONFIGMANAGER_H
