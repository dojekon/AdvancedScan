#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>

class Settings
{
public:
    static Settings& getInstance();
    
    // Настройки сканирования
    QString getDefaultColorMode() const;
    void setDefaultColorMode(const QString& mode);
    
    QString getDefaultResolution() const;
    void setDefaultResolution(const QString& resolution);
    
    QString getDefaultScanArea() const;
    void setDefaultScanArea(const QString& area);
    
    QString getDefaultOutputPath() const;
    void setDefaultOutputPath(const QString& path);
    
    QString getDefaultFilePrefix() const;
    void setDefaultFilePrefix(const QString& prefix);
    
    // Настройки приложения
    bool getAutoFindScanners() const;
    void setAutoFindScanners(bool enabled);
    
    bool getShowNotifications() const;
    void setShowNotifications(bool enabled);
    
    int getScanTimeout() const;
    void setScanTimeout(int timeout);
    
    // Настройки качества
    int getImageQuality() const;
    void setImageQuality(int quality);
    
    bool getAutoCrop() const;
    void setAutoCrop(bool enabled);
    
    // Методы для работы с настройками
    void loadSettings();
    void saveSettings();
    void resetToDefaults();
    
    // Получение списков опций
    QStringList getColorModes() const;
    QStringList getResolutions() const;
    QStringList getScanAreas() const;
    
    // Настройки вкладок
    QStringList getTabNames() const;
    void setTabNames(const QStringList& tabNames);
    
    // Настройки профилей сканирования
    QVariantMap getTabProfiles(const QString& tabName) const;
    void setTabProfiles(const QString& tabName, const QVariantMap& profiles);

private:
    Settings();
    ~Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    
    QSettings* m_settings;
    
    // Значения по умолчанию
    static const QString DEFAULT_COLOR_MODE;
    static const QString DEFAULT_RESOLUTION;
    static const QString DEFAULT_SCAN_AREA;
    static const QString DEFAULT_OUTPUT_PATH;
    static const QString DEFAULT_FILE_PREFIX;
    static const bool DEFAULT_AUTO_FIND_SCANNERS;
    static const bool DEFAULT_SHOW_NOTIFICATIONS;
    static const int DEFAULT_SCAN_TIMEOUT;
    static const int DEFAULT_IMAGE_QUALITY;
    static const bool DEFAULT_AUTO_CROP;
};

#endif // SETTINGS_H
