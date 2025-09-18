#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settingswindow.h"
#include "settings.h"
#include <iostream>
#include <qstring.h>
#include <cscanfront.h>
#include <string>
#include <cscanner.h>
#include <QThread>
#include <memory>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), activeDevice(nullptr), selectedDeviceIndex(-1), settingsWindow(nullptr) {
    ui->setupUi(this);

    // Подключаем кнопку настроек
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::on_pushButton_2_clicked);

    // Подключаем сигналы для вкладок
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(ui->tabWidget->tabBar(), &QTabBar::customContextMenuRequested, this, &MainWindow::showTabContextMenu);

    // Загружаем настройки
    loadSettings();

    // Инициализируем вкладки
    initializeTabs();

    // Показываем статус в статусбаре
    ui->statusbar->showMessage("Готов к работе");

    // Автоматически ищем сканеры если включено в настройках
    Settings& settings = Settings::getInstance();
    if (settings.getAutoFindScanners()) {
        // Запускаем поиск сканеров асинхронно через таймер
        QTimer::singleShot(100, this, &MainWindow::searchScannersAsync);
    }
}

MainWindow::~MainWindow() {  
    clearDevices();
    if (settingsWindow) {
        delete settingsWindow;
    }
    delete ui; 
}


void MainWindow::on_pushButton_clicked(){
    if (ui->comboBox->count() == 0 || !activeDevice) return;

    // Применяем настройки по умолчанию из конфигурации
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
    
    ui->statusbar->showMessage("Сканирование начато...");

    try {
        QImage image = CScanFront::scanImage(activeDevice);
        
        // Используем настройки для пути и имени файла
        Settings& settings = Settings::getInstance();
        QString outputPath = settings.getDefaultOutputPath();
        QString filePrefix = settings.getDefaultFilePrefix();
        QString extension = activeDevice->getExtension();
        
        // Создаем имя файла
        QString filename;
        if (!ui->lineEdit->text().isEmpty()) {
            filename = outputPath + "/" + ui->lineEdit->text() + extension;
        } else {
            filename = outputPath + "/" + filePrefix + QString::number(QDateTime::currentMSecsSinceEpoch()) + extension;
        }
        
        // Применяем настройки качества
        int quality = settings.getImageQuality();
        image.save(filename, nullptr, quality);
        
        QMessageBox::information(this, "Сканирование завершено", 
                               "Файл сохранен: " + filename);
    } catch(const std::exception& e) {
        QMessageBox::critical(this, "Ошибка сканирования", 
                            "Ошибка сканирования: " + QString::fromStdString(e.what()));
    } catch(...) {
        QMessageBox::critical(this, "Ошибка сканирования", 
                            "Неизвестная ошибка сканирования");
    }
}


void MainWindow::searchScannersAsync() {
    ui->statusbar->showMessage("Поиск сканеров...");
    ui->pushButton_3->setEnabled(false);
    
    // Запускаем поиск в отдельном потоке
    QThread* searchThread = QThread::create([this]() {
        std::vector<CScanner*> rawDevices = CScanFront::getDevices();
        
        // Передаем результат в главный поток
        QMetaObject::invokeMethod(this, [this, rawDevices]() {
            clearDevices();
            
            if (!rawDevices.empty()) {
                ui->comboBox->clear();
                
                // Переносим владение объектами в unique_ptr
                for (auto* scanner : rawDevices) {
                    devices.push_back(std::unique_ptr<CScanner>(scanner));
                }

                for (const auto& scanner : devices) {
                    ui->comboBox->addItem(scanner->getModel());
                }

                selectDevice(0); // Выбираем первый сканер по умолчанию
                ui->pushButton->setEnabled(true);
                ui->statusbar->showMessage("Найдено сканеров: " + QString::number(devices.size()));
            } else {
                ui->statusbar->showMessage("Сканеры не найдены");
            }
            
            ui->pushButton_3->setEnabled(true);
        }, Qt::QueuedConnection);
    });
    
    searchThread->start();
}

void MainWindow::on_pushButton_3_clicked() {
    searchScannersAsync();
}

void MainWindow::selectDevice(int deviceIndex) {
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        activeDevice = nullptr;
        selectedDeviceIndex = -1;
        return;
    }
    
    selectedDeviceIndex = deviceIndex;
    activeDevice = devices[selectedDeviceIndex].get();

    // Показываем расширение файла
    ui->label_5->setText(activeDevice->getExtension());
    
    // Применяем настройки по умолчанию из конфигурации
    applySettings();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    selectDevice(ui->comboBox->currentIndex());
}

void MainWindow::clearDevices() {
    devices.clear();
    activeDevice = nullptr;
    selectedDeviceIndex = -1;
}

void MainWindow::on_pushButton_2_clicked() {
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

void MainWindow::loadSettings() {
    Settings& settings = Settings::getInstance();
    
    // Устанавливаем значения по умолчанию из настроек
    ui->lineEdit->setPlaceholderText(settings.getDefaultFilePrefix());
}

void MainWindow::applySettings() {
    Settings& settings = Settings::getInstance();

    // Обновляем placeholder для имени файла
    ui->lineEdit->setPlaceholderText(settings.getDefaultFilePrefix());

    // Если есть активный сканер, применяем настройки по умолчанию
    if (activeDevice) {
        // Находим индексы для настроек по умолчанию
        QStringList colorModes = activeDevice->getSupportedColorModes();
        QStringList resolutions = activeDevice->getSupportedResolutions();
        QStringList scanAreas = activeDevice->getSupportedScanAreas();

        int colorIndex = colorModes.indexOf(settings.getDefaultColorMode());
        int resolutionIndex = resolutions.indexOf(settings.getDefaultResolution() + " DPI");
        int scanAreaIndex = scanAreas.indexOf(settings.getDefaultScanArea());

        // Устанавливаем настройки напрямую в сканер
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
}

void MainWindow::onTabChanged(int index) {
    // Игнорируем вкладку с плюсом - она обрабатывается в addPlusTab()
    if (index < 0 || index == plusTabIndex_) {
        return;
    }

    // Проверяем, что индекс находится в допустимых пределах
    if (index >= static_cast<int>(tabInfos.size())) {
        qDebug() << "Tab index out of range:" << index << ", tabInfos size:" << tabInfos.size();
        return;
    }

    // Загружаем информацию о выбранной вкладке
    const TabInfo& tabInfo = tabInfos[index];

    // Восстанавливаем выбранный сканер для этой вкладки
    if (tabInfo.deviceIndex >= 0 && tabInfo.deviceIndex < static_cast<int>(devices.size())) {
        selectDevice(tabInfo.deviceIndex);
    }

    ui->statusbar->showMessage("Выбрана вкладка: " + tabInfo.name);
}

void MainWindow::showTabContextMenu(const QPoint& pos) {
    // Определяем индекс вкладки по позиции курсора
    int tabIndex = ui->tabWidget->tabBar()->tabAt(pos);

    if (tabIndex >= 0) {
        contextMenuTabIndex = tabIndex;

        // Создаем контекстное меню
        QMenu* contextMenu = new QMenu(this);
        QAction* deleteAction = contextMenu->addAction("Удалить вкладку");

        connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteTab);

        // Показываем меню
        contextMenu->exec(ui->tabWidget->tabBar()->mapToGlobal(pos));
        delete contextMenu;
    }
}

void MainWindow::deleteTab() {
    if (contextMenuTabIndex >= 0 && contextMenuTabIndex < ui->tabWidget->count()) {
        // Не разрешаем удалять вкладку с плюсом
        if (contextMenuTabIndex == plusTabIndex_) {
            QMessageBox::information(this, "Удаление вкладки",
                                   "Нельзя удалить вкладку для создания новых вкладок");
            return;
        }

        // Не разрешаем удалять последнюю обычную вкладку
        if (static_cast<int>(tabInfos.size()) <= 1) {
            QMessageBox::information(this, "Удаление вкладки",
                                   "Нельзя удалить последнюю вкладку");
            return;
        }

        // Подтверждаем удаление
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Удаление вкладки",
            "Вы действительно хотите удалить эту вкладку?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            ui->tabWidget->removeTab(contextMenuTabIndex);
            tabInfos.erase(tabInfos.begin() + contextMenuTabIndex);

            // Если удаляемая вкладка была перед вкладкой с плюсом, обновляем индекс
            if (contextMenuTabIndex < plusTabIndex_) {
                plusTabIndex_--;
            }

            // Сохраняем изменения в конфиг
            saveTabs();

            qDebug() << "Deleted tab at index:" << contextMenuTabIndex << ", plus tab now at:" << plusTabIndex_;
            ui->statusbar->showMessage("Вкладка удалена");
        }
    }
}

void MainWindow::showTabContextMenuForProfiles(int tabIndex, const QPoint& pos) {
    if (tabIndex < 0 || tabIndex >= static_cast<int>(tabInfos.size())) {
        return;
    }

    TabInfo& tabInfo = tabInfos[tabIndex];

    // Проверяем лимит кнопок (максимум 5)
    if (tabInfo.profiles.size() >= 5) {
        QMessageBox::information(this, "Лимит профилей", 
                               "Максимальное количество профилей на вкладке: 5");
        return;
    }

    QMenu* contextMenu = new QMenu(this);
    QAction* createProfileAction = contextMenu->addAction("Создать профиль");

    connect(createProfileAction, &QAction::triggered, [this, tabIndex]() {
        showProfileDialog(tabIndex);
    });

    // Показываем меню
    QWidget* tabWidget = ui->tabWidget->widget(tabIndex);
    contextMenu->exec(tabWidget->mapToGlobal(pos));
    delete contextMenu;
}

void MainWindow::showProfileDialog(int tabIndex) {
    if (tabIndex < 0 || tabIndex >= static_cast<int>(tabInfos.size())) {
        return;
    }

    TabInfo& tabInfo = tabInfos[tabIndex];

    // Проверяем лимит кнопок
    if (tabInfo.profiles.size() >= 5) {
        QMessageBox::information(this, "Лимит профилей", 
                               "Максимальное количество профилей на вкладке: 5");
        return;
    }

    // Создаем диалог для создания нового профиля
    QDialog dialog(this);
    dialog.setWindowTitle("Создать профиль сканирования");
    dialog.setModal(true);
    dialog.resize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    // Поле для имени профиля
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Имя профиля:");
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Введите название профиля");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    mainLayout->addLayout(nameLayout);

    // Настройки сканирования
    QGroupBox* settingsGroup = new QGroupBox("Настройки сканирования");
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsGroup);

    // Режим цвета
    QHBoxLayout* colorLayout = new QHBoxLayout();
    QLabel* colorLabel = new QLabel("Режим цвета:");
    QComboBox* colorCombo = new QComboBox();
    if (activeDevice) {
        colorCombo->addItems(activeDevice->getSupportedColorModes());
    } else {
        colorCombo->addItems({"Цветной", "Черно-белый", "Оттенки серого"});
    }
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorCombo);
    settingsLayout->addLayout(colorLayout);

    // Разрешение
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    QLabel* resolutionLabel = new QLabel("Разрешение:");
    QComboBox* resolutionCombo = new QComboBox();
    if (activeDevice) {
        resolutionCombo->addItems(activeDevice->getSupportedResolutions());
    } else {
        resolutionCombo->addItems({"150 DPI", "300 DPI", "600 DPI"});
    }
    resolutionLayout->addWidget(resolutionLabel);
    resolutionLayout->addWidget(resolutionCombo);
    settingsLayout->addLayout(resolutionLayout);

    // Область сканирования
    QHBoxLayout* areaLayout = new QHBoxLayout();
    QLabel* areaLabel = new QLabel("Область сканирования:");
    QComboBox* areaCombo = new QComboBox();
    if (activeDevice) {
        areaCombo->addItems(activeDevice->getSupportedScanAreas());
    } else {
        areaCombo->addItems({"A4", "A3", "A5", "Letter"});
    }
    areaLayout->addWidget(areaLabel);
    areaLayout->addWidget(areaCombo);
    settingsLayout->addLayout(areaLayout);

    // Качество изображения
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    QLabel* qualityLabel = new QLabel("Качество:");
    QSpinBox* qualitySpin = new QSpinBox();
    qualitySpin->setRange(1, 100);
    qualitySpin->setValue(90);
    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(qualitySpin);
    settingsLayout->addLayout(qualityLayout);

    mainLayout->addWidget(settingsGroup);

    // Настройки файла
    QGroupBox* fileGroup = new QGroupBox("Настройки файла");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);

    // Префикс файла
    QHBoxLayout* prefixLayout = new QHBoxLayout();
    QLabel* prefixLabel = new QLabel("Префикс файла:");
    QLineEdit* prefixEdit = new QLineEdit();
    prefixEdit->setPlaceholderText("Например: Документ");
    prefixLayout->addWidget(prefixLabel);
    prefixLayout->addWidget(prefixEdit);
    fileLayout->addLayout(prefixLayout);

    // Формат названия файла
    QHBoxLayout* formatLayout = new QHBoxLayout();
    QLabel* formatLabel = new QLabel("Формат названия:");
    QComboBox* formatCombo = new QComboBox();
    formatCombo->addItem("Префикс_ДатаВремя", "PREFIX_DATETIME");
    formatCombo->addItem("Префикс_Дата", "PREFIX_DATE");
    formatCombo->addItem("Префикс_Время", "PREFIX_TIME");
    formatCombo->addItem("Префикс_Номер", "PREFIX_NUMBER");
    formatCombo->addItem("Только префикс", "PREFIX_ONLY");
    formatCombo->addItem("Только дата и время", "DATETIME_ONLY");
    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(formatCombo);
    fileLayout->addLayout(formatLayout);

    // Предварительный просмотр названия файла
    QHBoxLayout* previewLayout = new QHBoxLayout();
    QLabel* previewLabel = new QLabel("Предварительный просмотр:");
    QLabel* previewText = new QLabel("Документ_2024-01-15_14-30-25");
    previewText->setStyleSheet("color: #666; font-style: italic;");
    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(previewText);
    fileLayout->addLayout(previewLayout);

    // Функция обновления предварительного просмотра
    auto updatePreview = [=]() {
        QString prefix = prefixEdit->text().isEmpty() ? "Документ" : prefixEdit->text();
        QString format = formatCombo->currentData().toString();
        QString preview;
        
        QDateTime now = QDateTime::currentDateTime();
        
        if (format == "PREFIX_DATETIME") {
            preview = prefix + "_" + now.toString("yyyy-MM-dd_hh-mm-ss");
        } else if (format == "PREFIX_DATE") {
            preview = prefix + "_" + now.toString("yyyy-MM-dd");
        } else if (format == "PREFIX_TIME") {
            preview = prefix + "_" + now.toString("hh-mm-ss");
        } else if (format == "PREFIX_NUMBER") {
            preview = prefix + "_001";
        } else if (format == "PREFIX_ONLY") {
            preview = prefix;
        } else if (format == "DATETIME_ONLY") {
            preview = now.toString("yyyy-MM-dd_hh-mm-ss");
        }
        
        previewText->setText(preview);
    };

    connect(prefixEdit, &QLineEdit::textChanged, updatePreview);
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updatePreview);

    mainLayout->addWidget(fileGroup);

    // Кнопки OK/Cancel
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("Создать");
    QPushButton* cancelButton = new QPushButton("Отмена");

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    if (dialog.exec() == QDialog::Accepted && !nameEdit->text().isEmpty()) {
        // Создаем новый профиль
        ScanProfile newProfile;
        newProfile.name = nameEdit->text();
        newProfile.buttonText = newProfile.name;
        newProfile.colorMode = colorCombo->currentText();
        newProfile.resolution = resolutionCombo->currentText();
        newProfile.scanArea = areaCombo->currentText();
        newProfile.quality = qualitySpin->value();
        
        // Настройки файла
        newProfile.filePrefix = prefixEdit->text().isEmpty() ? "Документ" : prefixEdit->text();
        newProfile.fileFormat = formatCombo->currentData().toString();

        Settings& settings = Settings::getInstance();
        newProfile.outputPath = settings.getDefaultOutputPath();

        // Добавляем профиль в вкладку
        tabInfo.profiles.push_back(newProfile);

        // Добавляем кнопку профиля на вкладку
        addProfileButton(tabIndex, newProfile);

        // Сохраняем изменения в конфиг
        saveTabs();

        ui->statusbar->showMessage("Профиль создан: " + newProfile.name);
    }
}

void MainWindow::initializeTabs() {
    // Очищаем существующие вкладки кроме первой
    while (ui->tabWidget->count() > 1) {
        ui->tabWidget->removeTab(1);
    }
    tabInfos.clear();

    // Создаем начальную вкладку "Сканирование"
    TabInfo defaultTab;
    defaultTab.name = "Сканирование";
    defaultTab.deviceIndex = -1;
    tabInfos.push_back(defaultTab);

    // Настраиваем содержимое первой вкладки
    setupTabContent(0);

    // Загружаем сохранённые вкладки из конфига
    loadTabs();

    // Добавляем специальную вкладку с кнопкой "+"
    addPlusTab();
}

void MainWindow::addPlusTab() {
    // Создаем пустой виджет для вкладки с плюсом
    QWidget* plusTabWidget = new QWidget();
    
    // Добавляем вкладку с заголовком "+"
    int plusTabIndex = ui->tabWidget->addTab(plusTabWidget, "+");
    plusTabIndex_ = plusTabIndex;

    // Подключаем обработчик клика по заголовку вкладки
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == plusTabIndex_) {
            // Если выбрана вкладка с плюсом, открываем диалог и возвращаемся на предыдущую
            qDebug() << "Plus tab selected, opening dialog";
            createNewTabDialog();
            
            // Возвращаемся на предыдущую вкладку
            int prevIndex = (plusTabIndex_ > 0) ? plusTabIndex_ - 1 : 0;
            ui->tabWidget->setCurrentIndex(prevIndex);
        }
    });

    qDebug() << "Added plus tab at index:" << plusTabIndex;
}

void MainWindow::createNewTabDialog() {
    qDebug() << "createNewTabDialog called";
    
    QDialog dialog(this);
    dialog.setWindowTitle("Создать новую вкладку");
    dialog.setModal(true);
    dialog.resize(300, 150);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Поле для имени вкладки
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Имя вкладки:");
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setText("Новая вкладка");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    layout->addLayout(nameLayout);

    // Кнопки OK/Cancel
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Отмена");

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    qDebug() << "Showing dialog";
    
    if (dialog.exec() == QDialog::Accepted && !nameEdit->text().isEmpty()) {
        qDebug() << "Dialog accepted, creating tab:" << nameEdit->text();
        createNewTab(nameEdit->text());
    } else {
        qDebug() << "Dialog rejected or empty name";
    }
}

void MainWindow::loadTabs() {
    Settings& settings = Settings::getInstance();
    QStringList savedTabNames = settings.getTabNames();

    qDebug() << "Loading saved tabs:" << savedTabNames;

    // Загружаем каждую сохранённую вкладку
    for (const QString& tabName : savedTabNames) {
        TabInfo tabInfo;
        tabInfo.name = tabName;
        tabInfo.deviceIndex = -1;

        // Загружаем профили для этой вкладки
        QVariantMap profilesMap = settings.getTabProfiles(tabName);
        for (auto it = profilesMap.begin(); it != profilesMap.end(); ++it) {
            QVariantMap profileData = it.value().toMap();
            
            ScanProfile profile;
            profile.name = it.key();
            profile.colorMode = profileData.value("colorMode", "").toString();
            profile.resolution = profileData.value("resolution", "").toString();
            profile.scanArea = profileData.value("scanArea", "").toString();
            profile.outputPath = profileData.value("outputPath", "").toString();
            profile.filePrefix = profileData.value("filePrefix", "").toString();
            profile.fileFormat = profileData.value("fileFormat", "PREFIX_DATETIME").toString();
            profile.quality = profileData.value("quality", 90).toInt();
            profile.buttonText = profileData.value("buttonText", profile.name).toString();
            
            tabInfo.profiles.push_back(profile);
        }

        // Добавляем вкладку в список
        tabInfos.push_back(tabInfo);

        // Создаем вкладку в UI
        QWidget* tabWidget = new QWidget();
        int tabIndex = ui->tabWidget->addTab(tabWidget, tabName);
        
        // Настраиваем содержимое вкладки
        setupTabContent(tabIndex);

        qDebug() << "Loaded tab:" << tabName << "with" << tabInfo.profiles.size() << "profiles";
    }
}

void MainWindow::saveTabs() {
    Settings& settings = Settings::getInstance();
    
    // Собираем имена всех вкладок (кроме первой "Сканирование" и последней с плюсом)
    QStringList tabNames;
    for (int i = 1; i < static_cast<int>(tabInfos.size()); ++i) {
        tabNames.append(tabInfos[i].name);
    }

    // Сохраняем список имён вкладок
    settings.setTabNames(tabNames);

    // Сохраняем профили для каждой вкладки
    for (int i = 1; i < static_cast<int>(tabInfos.size()); ++i) {
        const TabInfo& tabInfo = tabInfos[i];
        
        QVariantMap profilesMap;
        for (const ScanProfile& profile : tabInfo.profiles) {
            QVariantMap profileData;
            profileData["colorMode"] = profile.colorMode;
            profileData["resolution"] = profile.resolution;
            profileData["scanArea"] = profile.scanArea;
            profileData["outputPath"] = profile.outputPath;
            profileData["filePrefix"] = profile.filePrefix;
            profileData["fileFormat"] = profile.fileFormat;
            profileData["quality"] = profile.quality;
            profileData["buttonText"] = profile.buttonText;
            
            profilesMap[profile.name] = profileData;
        }
        
        settings.setTabProfiles(tabInfo.name, profilesMap);
    }

    qDebug() << "Saved" << tabNames.size() << "tabs to config";
}

void MainWindow::createNewTab(const QString& tabName) {
    TabInfo newTabInfo;
    newTabInfo.name = tabName;
    newTabInfo.deviceIndex = selectedDeviceIndex;

    // Добавляем информацию о вкладке в вектор (перед последним элементом, если он есть)
    tabInfos.push_back(newTabInfo);

    // Создаем новую вкладку и вставляем перед вкладкой с плюсом
    QWidget* newTab = new QWidget();
    int insertIndex = plusTabIndex_; // Вставляем перед плюсом
    ui->tabWidget->insertTab(insertIndex, newTab, tabName);

    // Обновляем индекс вкладки с плюсом (сдвигается вправо)
    plusTabIndex_++;

    // Настраиваем содержимое вкладки
    setupTabContent(insertIndex);

    // Переключаемся на новую вкладку
    ui->tabWidget->setCurrentIndex(insertIndex);

    // Сохраняем изменения в конфиг
    saveTabs();

    qDebug() << "Created new tab at index:" << insertIndex << ", plus tab now at:" << plusTabIndex_;
}

void MainWindow::setupTabContent(int tabIndex) {
    if (tabIndex < 0 || tabIndex >= ui->tabWidget->count()) {
        return;
    }

    QWidget* tabWidget = ui->tabWidget->widget(tabIndex);
    if (!tabWidget) return;

    // Очищаем содержимое вкладки
    QLayout* existingLayout = tabWidget->layout();
    if (existingLayout) {
        QLayoutItem* item;
        while ((item = existingLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete existingLayout;
    }

    // Создаем горизонтальный layout для кнопок профилей
    QHBoxLayout* layout = new QHBoxLayout(tabWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Добавляем растягивающийся элемент для центрирования
    layout->addStretch();

    // Добавляем существующие профили
    if (tabIndex < static_cast<int>(tabInfos.size())) {
        for (const auto& profile : tabInfos[tabIndex].profiles) {
            addProfileButton(tabIndex, profile);
        }
    }

    // Добавляем растягивающийся элемент для центрирования
    layout->addStretch();

    // Включаем контекстное меню для вкладки
    tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabWidget, &QWidget::customContextMenuRequested, [this, tabIndex](const QPoint& pos) {
        showTabContextMenuForProfiles(tabIndex, pos);
    });
}

void MainWindow::addProfileButton(int tabIndex, const ScanProfile& profile) {
    if (tabIndex < 0 || tabIndex >= ui->tabWidget->count()) {
        return;
    }

    QWidget* tabWidget = ui->tabWidget->widget(tabIndex);
    if (!tabWidget || !tabWidget->layout()) return;

    QPushButton* profileButton = new QPushButton(profile.buttonText);
    profileButton->setFixedSize(120, 120); // Большие квадратные кнопки
    profileButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    padding: 8px;"
        "    text-align: center;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "    transform: scale(1.05);"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "    transform: scale(0.95);"
        "}"
    );

    connect(profileButton, &QPushButton::clicked, [this, profile]() {
        scanWithProfile(profile);
    });

    // Вставляем кнопку перед stretch элементом
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(tabWidget->layout());
    if (layout) {
        int insertIndex = layout->count() - 1; // Перед stretch
        layout->insertWidget(insertIndex, profileButton);
    }
}

void MainWindow::scanWithProfile(const ScanProfile& profile) {
    if (!activeDevice) {
        QMessageBox::warning(this, "Ошибка", "Не выбран сканер");
        return;
    }

    // Применяем настройки профиля
    QStringList colorModes = activeDevice->getSupportedColorModes();
    QStringList resolutions = activeDevice->getSupportedResolutions();
    QStringList scanAreas = activeDevice->getSupportedScanAreas();

    int colorIndex = colorModes.indexOf(profile.colorMode);
    int resolutionIndex = resolutions.indexOf(profile.resolution);
    int scanAreaIndex = scanAreas.indexOf(profile.scanArea);

    if (colorIndex >= 0) activeDevice->setSelectedColor(colorIndex);
    if (resolutionIndex >= 0) activeDevice->setSelectedResolution(resolutionIndex);
    if (scanAreaIndex >= 0) activeDevice->setSelectedScanArea(scanAreaIndex);

    ui->statusbar->showMessage("Сканирование с профилем: " + profile.name);

    try {
        QImage image = CScanFront::scanImage(activeDevice);

        // Генерируем имя файла согласно настройкам профиля
        QString filename = generateFileName(profile);

        image.save(filename, nullptr, profile.quality);

        QMessageBox::information(this, "Сканирование завершено",
                               "Файл сохранен: " + filename);
    } catch(const std::exception& e) {
        QMessageBox::critical(this, "Ошибка сканирования",
                            "Ошибка сканирования: " + QString::fromStdString(e.what()));
    } catch(...) {
        QMessageBox::critical(this, "Ошибка сканирования",
                            "Неизвестная ошибка сканирования");
    }
}

QString MainWindow::generateFileName(const ScanProfile& profile) {
    QString filename;
    QDateTime now = QDateTime::currentDateTime();
    
    if (profile.fileFormat == "PREFIX_DATETIME") {
        filename = profile.filePrefix + "_" + now.toString("yyyy-MM-dd_hh-mm-ss");
    } else if (profile.fileFormat == "PREFIX_DATE") {
        filename = profile.filePrefix + "_" + now.toString("yyyy-MM-dd");
    } else if (profile.fileFormat == "PREFIX_TIME") {
        filename = profile.filePrefix + "_" + now.toString("hh-mm-ss");
    } else if (profile.fileFormat == "PREFIX_NUMBER") {
        // Генерируем уникальный номер
        static int fileCounter = 1;
        filename = profile.filePrefix + "_" + QString::number(fileCounter++).rightJustified(3, '0');
    } else if (profile.fileFormat == "PREFIX_ONLY") {
        filename = profile.filePrefix;
    } else if (profile.fileFormat == "DATETIME_ONLY") {
        filename = now.toString("yyyy-MM-dd_hh-mm-ss");
    } else {
        // По умолчанию используем префикс + дата и время
        filename = profile.filePrefix + "_" + now.toString("yyyy-MM-dd_hh-mm-ss");
    }
    
    return profile.outputPath + "/" + filename + activeDevice->getExtension();
}

