#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settingswindow.h"
#include "../core/settings.h"
#include "../managers/scannermanager.h"
#include "../managers/profilemanager.h"
#include "../managers/tabmanager.h"
#include "../managers/filemanager.h"
#include "../core/cscanfront.h"
#include <iostream>
#include <qstring.h>
#include <string>
#include <QThread>
#include <memory>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settingsWindow(nullptr)
    , scannerManager(nullptr)
    , profileManager(nullptr)
    , tabManager(nullptr)
    , fileManager(nullptr)
{
    ui->setupUi(this);

    // Инициализируем менеджеры
    scannerManager = new ScannerManager(this);
    profileManager = new ProfileManager(this);
    tabManager = new TabManager(ui->tabWidget, this);
    fileManager = new FileManager(this);
    
    // Устанавливаем связи между менеджерами
    profileManager->setScannerManager(scannerManager);

    // Настраиваем соединения
    setupConnections();

    // Подключаем кнопку настроек
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::on_pushButton_2_clicked);

    // Загружаем настройки
    loadSettings();

    // Инициализируем вкладки
    tabManager->initializeTabs();

    // Показываем статус в статусбаре
    updateStatusBar("Готов к работе");

    // Автоматически ищем сканеры если включено в настройках
    Settings& settings = Settings::getInstance();
    if (settings.getAutoFindScanners()) {
        // Запускаем поиск сканеров асинхронно через таймер
        QTimer::singleShot(100, this, &MainWindow::on_pushButton_3_clicked);
    }
}

MainWindow::~MainWindow() {  
    if (settingsWindow) {
        delete settingsWindow;
    }
    delete ui; 
}

void MainWindow::setupConnections()
{
    // Соединения с ScannerManager
    connect(scannerManager, &ScannerManager::scannersFound, this, &MainWindow::onScannersFound);
    connect(scannerManager, &ScannerManager::scannerSelected, this, &MainWindow::onScannerSelected);
    connect(scannerManager, &ScannerManager::searchStarted, this, [this]() {
        updateStatusBar("Поиск сканеров...");
        ui->pushButton_3->setEnabled(false);
    });
    connect(scannerManager, &ScannerManager::searchFinished, this, [this]() {
        ui->pushButton_3->setEnabled(true);
    });

    // Соединения с TabManager
    connect(tabManager, &TabManager::tabChanged, this, &MainWindow::onTabChanged);
    connect(tabManager, &TabManager::profileButtonClicked, this, &MainWindow::onProfileButtonClicked);
    connect(tabManager, &TabManager::profileContextMenuRequested, this, &MainWindow::onProfileContextMenuRequested);

    // Соединения с ProfileManager
    connect(profileManager, &ProfileManager::profileCreated, this, [this](int tabIndex, const ProfileManager::ScanProfile& profile) {
        tabManager->addProfileToTab(tabIndex, profile);
        updateStatusBar("Профиль создан: " + profile.name);
    });
    connect(profileManager, &ProfileManager::profileUpdated, this, [this](int tabIndex, const ProfileManager::ScanProfile& profile) {
        tabManager->updateProfileInTab(tabIndex, profile);
        updateStatusBar("Профиль обновлен: " + profile.name);
    });
    connect(profileManager, &ProfileManager::profileDeleted, this, [this](int tabIndex, const QString& profileName) {
        tabManager->removeProfileFromTab(tabIndex, profileName);
        updateStatusBar("Профиль удален: " + profileName);
    });

    // Соединения с FileManager
    connect(fileManager, &FileManager::fileSaved, this, &MainWindow::onFileSaved);
    connect(fileManager, &FileManager::fileSaveError, this, &MainWindow::onFileSaveError);
}

void MainWindow::on_pushButton_clicked()
{
    if (!scannerManager->hasDevices() || !scannerManager->getActiveDevice()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран сканер");
        return;
    }

    // Применяем настройки по умолчанию
    scannerManager->applyDefaultSettings();
    
    // Сбрасываем пользовательскую область для обычного сканирования
    scannerManager->getActiveDevice()->resetCustomScanArea();
    
    updateStatusBar("Сканирование начато...");

    try {
        QImage image = CScanFront::scanImage(scannerManager->getActiveDevice());
        
        // Используем настройки для пути и имени файла
        Settings& settings = Settings::getInstance();
        QString outputPath = settings.getDefaultOutputPath();
        QString filePrefix = settings.getDefaultFilePrefix();
        QString extension = scannerManager->getFileExtension();
        
        // Создаем имя файла
        QString filename;
        if (!ui->lineEdit->text().isEmpty()) {
            filename = outputPath + "/" + ui->lineEdit->text() + extension;
        } else {
            filename = outputPath + "/" + filePrefix + QString::number(QDateTime::currentMSecsSinceEpoch()) + extension;
        }
        
        // Применяем настройки качества
        int quality = settings.getImageQuality();
        fileManager->saveImage(image, filename, quality);
        
    } catch(const std::exception& e) {
        QMessageBox::critical(this, "Ошибка сканирования", 
                            "Ошибка сканирования: " + QString::fromStdString(e.what()));
    } catch(...) {
        QMessageBox::critical(this, "Ошибка сканирования", 
                            "Неизвестная ошибка сканирования");
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    scannerManager->searchScannersAsync();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    scannerManager->selectDevice(index);
}

void MainWindow::on_pushButton_2_clicked()
{
    // Создаем окно настроек только один раз
    if (!settingsWindow) {
        settingsWindow = new SettingsWindow(this);
        // Подключаем сигнал finished для обработки закрытия
        connect(settingsWindow, &QDialog::finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                applySettings();
            }
        });
    }
    
    // Показываем окно
    settingsWindow->show();
    settingsWindow->raise();
    settingsWindow->activateWindow();
}

void MainWindow::onTabChanged(int index)
{
    // Загружаем информацию о выбранной вкладке
    TabManager::TabInfo* tabInfo = tabManager->getTabInfo(index);
    if (tabInfo) {
        // Восстанавливаем выбранный сканер для этой вкладки
        if (tabInfo->deviceIndex >= 0 && tabInfo->deviceIndex < static_cast<int>(scannerManager->getDevices().size())) {
            scannerManager->selectDevice(tabInfo->deviceIndex);
        }
        updateStatusBar("Выбрана вкладка: " + tabInfo->name);
    }
}

void MainWindow::onProfileButtonClicked(const ProfileManager::ScanProfile& profile)
{
    scanWithProfile(profile);
}

void MainWindow::onProfileContextMenuRequested(int tabIndex, const ProfileManager::ScanProfile& profile, const QPoint& pos)
{
    if (tabIndex >= 0) {
        // Создание нового профиля
        QStringList colorModes = scannerManager->getSupportedColorModes();
        QStringList resolutions = scannerManager->getSupportedResolutions();
        QStringList scanAreas = scannerManager->getSupportedScanAreas();
        
        ProfileManager::ScanProfile newProfile = profileManager->showProfileDialogWithResult(tabIndex, colorModes, resolutions, scanAreas);
        if (!newProfile.name.isEmpty()) {
            emit profileManager->profileCreated(tabIndex, newProfile);
        }
    } else {
        // Редактирование существующего профиля
        QStringList colorModes = scannerManager->getSupportedColorModes();
        QStringList resolutions = scannerManager->getSupportedResolutions();
        QStringList scanAreas = scannerManager->getSupportedScanAreas();
        
        profileManager->showProfileContextMenu(tabIndex, profile, pos);
    }
}

void MainWindow::onScannersFound()
{
    updateScannerComboBox();
    updateStatusBar("Найдено сканеров: " + QString::number(scannerManager->getDevices().size()));
    ui->pushButton->setEnabled(true);
}

void MainWindow::onScannerSelected()
{
    // Обновляем информацию о выбранном сканере
    if (scannerManager->getActiveDevice()) {
        ui->label_5->setText(scannerManager->getFileExtension());
    }
}

void MainWindow::onFileSaved(const QString& filename)
{
    QMessageBox::information(this, "Сканирование завершено", 
                           "Файл сохранен: " + filename);
    updateStatusBar("Файл сохранен: " + filename);
}

void MainWindow::onFileSaveError(const QString& error)
{
    QMessageBox::critical(this, "Ошибка сохранения", error);
    updateStatusBar("Ошибка: " + error);
}

void MainWindow::loadSettings()
{
    Settings& settings = Settings::getInstance();
    
    // Устанавливаем значения по умолчанию из настроек
    ui->lineEdit->setPlaceholderText(settings.getDefaultFilePrefix());
}

void MainWindow::applySettings()
{
    Settings& settings = Settings::getInstance();

    // Обновляем placeholder для имени файла
    ui->lineEdit->setPlaceholderText(settings.getDefaultFilePrefix());

    // Применяем настройки к активному сканеру
    scannerManager->applyDefaultSettings();
}

void MainWindow::scanWithProfile(const ProfileManager::ScanProfile& profile)
{
    if (!scannerManager->getActiveDevice()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран сканер");
        return;
    }

    // Применяем настройки профиля к сканеру
    QStringList colorModes = scannerManager->getSupportedColorModes();
    QStringList resolutions = scannerManager->getSupportedResolutions();
    QStringList scanAreas = scannerManager->getSupportedScanAreas();

    int colorIndex = colorModes.indexOf(profile.colorMode);
    int resolutionIndex = resolutions.indexOf(profile.resolution);
    int scanAreaIndex = scanAreas.indexOf(profile.scanArea);

    if (colorIndex >= 0) scannerManager->getActiveDevice()->setSelectedColor(colorIndex);
    if (resolutionIndex >= 0) scannerManager->getActiveDevice()->setSelectedResolution(resolutionIndex);
    if (scanAreaIndex >= 0) scannerManager->getActiveDevice()->setSelectedScanArea(scanAreaIndex);
    
    // Если есть пользовательская область, устанавливаем её в сканер
    if (profile.useCustomArea && !profile.customArea.isEmpty()) {
        qDebug() << "MainWindow: Setting custom area:" << profile.customArea;
        scannerManager->getActiveDevice()->setCustomScanArea(profile.customArea);
    } else {
        qDebug() << "MainWindow: No custom area set, useCustomArea:" << profile.useCustomArea;
    }

    updateStatusBar("Сканирование с профилем: " + profile.name);

    try {
        QImage image = CScanFront::scanImage(scannerManager->getActiveDevice());

        // Генерируем имя файла согласно настройкам профиля
        QString filename = fileManager->generateFileName(profile);

        // Сохраняем изображение в выбранном формате
        if (profile.outputFormat == "PDF") {
            fileManager->saveImageAsPDF(image, filename, profile.quality, profile.resolution, profile.scanArea);
        } else {
            fileManager->saveImage(image, filename, profile.quality);
        }

    } catch(const std::exception& e) {
        QMessageBox::critical(this, "Ошибка сканирования",
                            "Ошибка сканирования: " + QString::fromStdString(e.what()));
    } catch(...) {
        QMessageBox::critical(this, "Ошибка сканирования",
                            "Неизвестная ошибка сканирования");
    }
}

void MainWindow::updateScannerComboBox()
{
    ui->comboBox->clear();
    
    for (const auto& scanner : scannerManager->getDevices()) {
        ui->comboBox->addItem(scanner->getModel());
    }
    
    // Выбираем первый сканер по умолчанию
    if (!scannerManager->getDevices().empty()) {
        scannerManager->selectDevice(0);
    }
}

void MainWindow::updateStatusBar(const QString& message)
{
    ui->statusbar->showMessage(message);
}

