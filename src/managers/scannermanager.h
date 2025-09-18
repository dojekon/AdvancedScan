#ifndef SCANNERMANAGER_H
#define SCANNERMANAGER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <vector>
#include <memory>
#include "../core/cscanner.h"

class ScannerManager : public QObject
{
    Q_OBJECT

public:
    explicit ScannerManager(QObject *parent = nullptr);
    ~ScannerManager();

    // Основные методы
    void searchScannersAsync();
    void clearDevices();
    void selectDevice(int deviceIndex);
    
    // Геттеры
    const std::vector<std::unique_ptr<CScanner>>& getDevices() const { return devices; }
    CScanner* getActiveDevice() const { return activeDevice; }
    int getSelectedDeviceIndex() const { return selectedDeviceIndex; }
    bool hasDevices() const { return !devices.empty(); }
    
    // Методы для работы с настройками сканера
    void applyDefaultSettings();
    QStringList getSupportedColorModes() const;
    QStringList getSupportedResolutions() const;
    QStringList getSupportedScanAreas() const;
    QString getFileExtension() const;

signals:
    void scannersFound();
    void scannerSelected();
    void searchStarted();
    void searchFinished();


private:
    std::vector<std::unique_ptr<CScanner>> devices;
    CScanner* activeDevice;
    int selectedDeviceIndex;
    
    void setupDevice(CScanner* device);
};

#endif // SCANNERMANAGER_H
