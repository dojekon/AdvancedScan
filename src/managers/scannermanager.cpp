#include "scannermanager.h"
#include "../core/cscanfront.h"
#include "../core/settings.h"
#include <QDebug>

ScannerManager::ScannerManager(QObject *parent)
    : QObject(parent)
    , activeDevice(nullptr)
    , selectedDeviceIndex(-1)
{
}

ScannerManager::~ScannerManager()
{
    clearDevices();
}

void ScannerManager::searchScannersAsync()
{
    emit searchStarted();
    
    // Запускаем поиск в отдельном потоке
    QThread* searchThread = QThread::create([this]() {
        std::vector<CScanner*> rawDevices = CScanFront::getDevices();
        
        // Передаем результат в главный поток
        QMetaObject::invokeMethod(this, [this, rawDevices]() {
            clearDevices();
            
            if (!rawDevices.empty()) {
                // Переносим владение объектами в unique_ptr
                for (auto* scanner : rawDevices) {
                    devices.push_back(std::unique_ptr<CScanner>(scanner));
                }
                
                // Выбираем первый сканер по умолчанию
                if (!devices.empty()) {
                    selectDevice(0);
                }
                
                emit scannersFound();
            }
            
            emit searchFinished();
        }, Qt::QueuedConnection);
    });
    
    searchThread->start();
}

void ScannerManager::clearDevices()
{
    devices.clear();
    activeDevice = nullptr;
    selectedDeviceIndex = -1;
}

void ScannerManager::selectDevice(int deviceIndex)
{
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        activeDevice = nullptr;
        selectedDeviceIndex = -1;
        return;
    }
    
    selectedDeviceIndex = deviceIndex;
    activeDevice = devices[selectedDeviceIndex].get();
    
    // Применяем настройки по умолчанию
    applyDefaultSettings();
    
    emit scannerSelected();
}

void ScannerManager::applyDefaultSettings()
{
    if (!activeDevice) return;
    
    Settings& settings = Settings::getInstance();
    
    // Находим индексы для настроек по умолчанию
    QStringList colorModes = activeDevice->getSupportedColorModes();
    QStringList resolutions = activeDevice->getSupportedResolutions();
    QStringList scanAreas = activeDevice->getSupportedScanAreas();
    
    int colorIndex = colorModes.indexOf(settings.getDefaultColorMode());
    int resolutionIndex = resolutions.indexOf(settings.getDefaultResolution() + " DPI");
    int scanAreaIndex = scanAreas.indexOf(settings.getDefaultScanArea());
    
    // Устанавливаем настройки по умолчанию
    if (colorIndex >= 0) {
        activeDevice->setSelectedColor(colorIndex);
    }
    if (resolutionIndex >= 0) {
        activeDevice->setSelectedResolution(resolutionIndex);
    }
    if (scanAreaIndex >= 0) {
        activeDevice->setSelectedScanArea(scanAreaIndex);
    }
}

QStringList ScannerManager::getSupportedColorModes() const
{
    return activeDevice ? activeDevice->getSupportedColorModes() : QStringList();
}

QStringList ScannerManager::getSupportedResolutions() const
{
    return activeDevice ? activeDevice->getSupportedResolutions() : QStringList();
}

QStringList ScannerManager::getSupportedScanAreas() const
{
    return activeDevice ? activeDevice->getSupportedScanAreas() : QStringList();
}

QString ScannerManager::getFileExtension() const
{
    return activeDevice ? activeDevice->getExtension() : QString();
}
