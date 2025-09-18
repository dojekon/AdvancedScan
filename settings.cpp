#include "settings.h"
#include <QApplication>
#include <QDebug>

// Статические константы для значений по умолчанию
const QString Settings::DEFAULT_COLOR_MODE = "Color";
const QString Settings::DEFAULT_RESOLUTION = "300";
const QString Settings::DEFAULT_SCAN_AREA = "A4 (210x297mm)";
const QString Settings::DEFAULT_OUTPUT_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
const QString Settings::DEFAULT_FILE_PREFIX = "scan_";
const bool Settings::DEFAULT_AUTO_FIND_SCANNERS = true;
const bool Settings::DEFAULT_SHOW_NOTIFICATIONS = true;
const int Settings::DEFAULT_SCAN_TIMEOUT = 30000; // 30 секунд
const int Settings::DEFAULT_IMAGE_QUALITY = 95;
const bool Settings::DEFAULT_AUTO_CROP = false;

Settings::Settings()
{
    // Создаем директорию для конфигурации
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/AdvancedScan";
    QDir().mkpath(configDir);
    
    // Инициализируем QSettings с путем к конфигурационному файлу
    QString configPath = configDir + "/settings.cfg";
    m_settings = new QSettings(configPath, QSettings::IniFormat);
    
    loadSettings();
}

Settings& Settings::getInstance()
{
    static Settings instance;
    return instance;
}

void Settings::loadSettings()
{
    // Загружаем настройки или устанавливаем значения по умолчанию
    m_settings->beginGroup("Scanning");
    m_settings->setValue("defaultColorMode", m_settings->value("defaultColorMode", DEFAULT_COLOR_MODE).toString());
    m_settings->setValue("defaultResolution", m_settings->value("defaultResolution", DEFAULT_RESOLUTION).toString());
    m_settings->setValue("defaultScanArea", m_settings->value("defaultScanArea", DEFAULT_SCAN_AREA).toString());
    m_settings->setValue("defaultOutputPath", m_settings->value("defaultOutputPath", DEFAULT_OUTPUT_PATH).toString());
    m_settings->setValue("defaultFilePrefix", m_settings->value("defaultFilePrefix", DEFAULT_FILE_PREFIX).toString());
    m_settings->endGroup();
    
    m_settings->beginGroup("Application");
    m_settings->setValue("autoFindScanners", m_settings->value("autoFindScanners", DEFAULT_AUTO_FIND_SCANNERS).toBool());
    m_settings->setValue("showNotifications", m_settings->value("showNotifications", DEFAULT_SHOW_NOTIFICATIONS).toBool());
    m_settings->setValue("scanTimeout", m_settings->value("scanTimeout", DEFAULT_SCAN_TIMEOUT).toInt());
    m_settings->endGroup();
    
    m_settings->beginGroup("Quality");
    m_settings->setValue("imageQuality", m_settings->value("imageQuality", DEFAULT_IMAGE_QUALITY).toInt());
    m_settings->setValue("autoCrop", m_settings->value("autoCrop", DEFAULT_AUTO_CROP).toBool());
    m_settings->endGroup();
}

void Settings::saveSettings()
{
    m_settings->sync();
}

void Settings::resetToDefaults()
{
    m_settings->clear();
    loadSettings();
}

// Геттеры для настроек сканирования
QString Settings::getDefaultColorMode() const
{
    return m_settings->value("Scanning/defaultColorMode", DEFAULT_COLOR_MODE).toString();
}

void Settings::setDefaultColorMode(const QString& mode)
{
    m_settings->setValue("Scanning/defaultColorMode", mode);
}

QString Settings::getDefaultResolution() const
{
    return m_settings->value("Scanning/defaultResolution", DEFAULT_RESOLUTION).toString();
}

void Settings::setDefaultResolution(const QString& resolution)
{
    m_settings->setValue("Scanning/defaultResolution", resolution);
}

QString Settings::getDefaultScanArea() const
{
    return m_settings->value("Scanning/defaultScanArea", DEFAULT_SCAN_AREA).toString();
}

void Settings::setDefaultScanArea(const QString& area)
{
    m_settings->setValue("Scanning/defaultScanArea", area);
}

QString Settings::getDefaultOutputPath() const
{
    return m_settings->value("Scanning/defaultOutputPath", DEFAULT_OUTPUT_PATH).toString();
}

void Settings::setDefaultOutputPath(const QString& path)
{
    m_settings->setValue("Scanning/defaultOutputPath", path);
}

QString Settings::getDefaultFilePrefix() const
{
    return m_settings->value("Scanning/defaultFilePrefix", DEFAULT_FILE_PREFIX).toString();
}

void Settings::setDefaultFilePrefix(const QString& prefix)
{
    m_settings->setValue("Scanning/defaultFilePrefix", prefix);
}

// Геттеры для настроек приложения
bool Settings::getAutoFindScanners() const
{
    return m_settings->value("Application/autoFindScanners", DEFAULT_AUTO_FIND_SCANNERS).toBool();
}

void Settings::setAutoFindScanners(bool enabled)
{
    m_settings->setValue("Application/autoFindScanners", enabled);
}

bool Settings::getShowNotifications() const
{
    return m_settings->value("Application/showNotifications", DEFAULT_SHOW_NOTIFICATIONS).toBool();
}

void Settings::setShowNotifications(bool enabled)
{
    m_settings->setValue("Application/showNotifications", enabled);
}

int Settings::getScanTimeout() const
{
    return m_settings->value("Application/scanTimeout", DEFAULT_SCAN_TIMEOUT).toInt();
}

void Settings::setScanTimeout(int timeout)
{
    m_settings->setValue("Application/scanTimeout", timeout);
}

// Геттеры для настроек качества
int Settings::getImageQuality() const
{
    return m_settings->value("Quality/imageQuality", DEFAULT_IMAGE_QUALITY).toInt();
}

void Settings::setImageQuality(int quality)
{
    m_settings->setValue("Quality/imageQuality", quality);
}

bool Settings::getAutoCrop() const
{
    return m_settings->value("Quality/autoCrop", DEFAULT_AUTO_CROP).toBool();
}

void Settings::setAutoCrop(bool enabled)
{
    m_settings->setValue("Quality/autoCrop", enabled);
}

// Методы для получения списков опций
QStringList Settings::getColorModes() const
{
    return QStringList{"Color", "Gray", "Lineart", "24bit Color", "True Gray", "Black & White", "Gray[Error Diffusion]"};
}

QStringList Settings::getResolutions() const
{
    return QStringList{"100", "200", "300", "400", "600", "1200"};
}

QStringList Settings::getScanAreas() const
{
    return QStringList{"Maximum Area", "A4 (210x297mm)", "Letter (8.5x11 in)", "4R (4x6in)"};
}

QStringList Settings::getTabNames() const
{
    return m_settings->value("tabs/names", QStringList()).toStringList();
}

void Settings::setTabNames(const QStringList& tabNames)
{
    m_settings->setValue("tabs/names", tabNames);
    m_settings->sync();
}

QVariantMap Settings::getTabProfiles(const QString& tabName) const
{
    return m_settings->value("profiles/" + tabName, QVariantMap()).toMap();
}

void Settings::setTabProfiles(const QString& tabName, const QVariantMap& profiles)
{
    m_settings->setValue("profiles/" + tabName, profiles);
    m_settings->sync();
}
