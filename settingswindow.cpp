#include "settingswindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent)
    , m_settings(&Settings::getInstance())
{
    setupUI();
    loadSettings();
    
    // Настройки окна
    setWindowTitle("Настройки AdvancedScan");
    setModal(true);
    setFixedSize(500, 400);
}

SettingsWindow::~SettingsWindow()
{
}

void SettingsWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Создаем вкладки
    m_tabWidget = new QTabWidget(this);
    
    // Вкладка "Сканирование"
    m_scanningTab = new QWidget();
    QVBoxLayout *scanningLayout = new QVBoxLayout(m_scanningTab);
    
    // Группа "Настройки сканирования"
    QGroupBox *scanningGroup = new QGroupBox("Настройки сканирования");
    QGridLayout *scanningGrid = new QGridLayout(scanningGroup);
    
    // Цветовой режим
    scanningGrid->addWidget(new QLabel("Цветовой режим по умолчанию:"), 0, 0);
    m_colorModeCombo = new QComboBox();
    m_colorModeCombo->addItems(m_settings->getColorModes());
    scanningGrid->addWidget(m_colorModeCombo, 0, 1);
    
    // Разрешение
    scanningGrid->addWidget(new QLabel("Разрешение по умолчанию:"), 1, 0);
    m_resolutionCombo = new QComboBox();
    m_resolutionCombo->addItems(m_settings->getResolutions());
    scanningGrid->addWidget(m_resolutionCombo, 1, 1);
    
    // Область сканирования
    scanningGrid->addWidget(new QLabel("Область сканирования по умолчанию:"), 2, 0);
    m_scanAreaCombo = new QComboBox();
    m_scanAreaCombo->addItems(m_settings->getScanAreas());
    scanningGrid->addWidget(m_scanAreaCombo, 2, 1);
    
    // Путь сохранения
    scanningGrid->addWidget(new QLabel("Папка для сохранения:"), 3, 0);
    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_outputPathEdit = new QLineEdit();
    m_browseButton = new QPushButton("Обзор...");
    connect(m_browseButton, &QPushButton::clicked, this, &SettingsWindow::onBrowseOutputPath);
    pathLayout->addWidget(m_outputPathEdit);
    pathLayout->addWidget(m_browseButton);
    scanningGrid->addLayout(pathLayout, 3, 1);
    
    // Префикс файла
    scanningGrid->addWidget(new QLabel("Префикс имени файла:"), 4, 0);
    m_filePrefixEdit = new QLineEdit();
    scanningGrid->addWidget(m_filePrefixEdit, 4, 1);
    
    scanningLayout->addWidget(scanningGroup);
    scanningLayout->addStretch();
    
    // Вкладка "Приложение"
    m_applicationTab = new QWidget();
    QVBoxLayout *applicationLayout = new QVBoxLayout(m_applicationTab);
    
    QGroupBox *applicationGroup = new QGroupBox("Настройки приложения");
    QGridLayout *applicationGrid = new QGridLayout(applicationGroup);
    
    // Автопоиск сканеров
    m_autoFindScannersCheck = new QCheckBox("Автоматически искать сканеры при запуске");
    applicationGrid->addWidget(m_autoFindScannersCheck, 0, 0, 1, 2);
    
    // Показывать уведомления
    m_showNotificationsCheck = new QCheckBox("Показывать уведомления");
    applicationGrid->addWidget(m_showNotificationsCheck, 1, 0, 1, 2);
    
    // Таймаут сканирования
    applicationGrid->addWidget(new QLabel("Таймаут сканирования (мс):"), 2, 0);
    m_scanTimeoutSpin = new QSpinBox();
    m_scanTimeoutSpin->setRange(5000, 120000);
    m_scanTimeoutSpin->setSuffix(" мс");
    applicationGrid->addWidget(m_scanTimeoutSpin, 2, 1);
    
    applicationLayout->addWidget(applicationGroup);
    applicationLayout->addStretch();
    
    // Вкладка "Качество"
    m_qualityTab = new QWidget();
    QVBoxLayout *qualityLayout = new QVBoxLayout(m_qualityTab);
    
    QGroupBox *qualityGroup = new QGroupBox("Настройки качества");
    QGridLayout *qualityGrid = new QGridLayout(qualityGroup);
    
    // Качество изображения
    qualityGrid->addWidget(new QLabel("Качество изображения:"), 0, 0);
    QHBoxLayout *qualitySliderLayout = new QHBoxLayout();
    m_imageQualitySlider = new QSlider(Qt::Horizontal);
    m_imageQualitySlider->setRange(50, 100);
    m_imageQualitySlider->setValue(95);
    connect(m_imageQualitySlider, &QSlider::valueChanged, this, &SettingsWindow::onImageQualityChanged);
    m_qualityValueLabel = new QLabel("95%");
    qualitySliderLayout->addWidget(m_imageQualitySlider);
    qualitySliderLayout->addWidget(m_qualityValueLabel);
    qualityGrid->addLayout(qualitySliderLayout, 0, 1);
    
    // Автообрезка
    m_autoCropCheck = new QCheckBox("Автоматическая обрезка изображений");
    qualityGrid->addWidget(m_autoCropCheck, 1, 0, 1, 2);
    
    qualityLayout->addWidget(qualityGroup);
    qualityLayout->addStretch();
    
    // Добавляем вкладки
    m_tabWidget->addTab(m_scanningTab, "Сканирование");
    m_tabWidget->addTab(m_applicationTab, "Приложение");
    m_tabWidget->addTab(m_qualityTab, "Качество");
    
    mainLayout->addWidget(m_tabWidget);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Отмена");
    m_resetButton = new QPushButton("Сбросить");
    
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    
    // Подключаем кнопки
    connect(okButton, &QPushButton::clicked, this, &SettingsWindow::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsWindow::reject);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsWindow::onResetClicked);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsWindow::loadSettings()
{
    // Загружаем настройки сканирования
    m_colorModeCombo->setCurrentText(m_settings->getDefaultColorMode());
    m_resolutionCombo->setCurrentText(m_settings->getDefaultResolution());
    m_scanAreaCombo->setCurrentText(m_settings->getDefaultScanArea());
    m_outputPathEdit->setText(m_settings->getDefaultOutputPath());
    m_filePrefixEdit->setText(m_settings->getDefaultFilePrefix());
    
    // Загружаем настройки приложения
    m_autoFindScannersCheck->setChecked(m_settings->getAutoFindScanners());
    m_showNotificationsCheck->setChecked(m_settings->getShowNotifications());
    m_scanTimeoutSpin->setValue(m_settings->getScanTimeout());
    
    // Загружаем настройки качества
    m_imageQualitySlider->setValue(m_settings->getImageQuality());
    m_qualityValueLabel->setText(QString::number(m_settings->getImageQuality()) + "%");
    m_autoCropCheck->setChecked(m_settings->getAutoCrop());
}

void SettingsWindow::saveSettings()
{
    // Сохраняем настройки сканирования
    m_settings->setDefaultColorMode(m_colorModeCombo->currentText());
    m_settings->setDefaultResolution(m_resolutionCombo->currentText());
    m_settings->setDefaultScanArea(m_scanAreaCombo->currentText());
    m_settings->setDefaultOutputPath(m_outputPathEdit->text());
    m_settings->setDefaultFilePrefix(m_filePrefixEdit->text());
    
    // Сохраняем настройки приложения
    m_settings->setAutoFindScanners(m_autoFindScannersCheck->isChecked());
    m_settings->setShowNotifications(m_showNotificationsCheck->isChecked());
    m_settings->setScanTimeout(m_scanTimeoutSpin->value());
    
    // Сохраняем настройки качества
    m_settings->setImageQuality(m_imageQualitySlider->value());
    m_settings->setAutoCrop(m_autoCropCheck->isChecked());
    
    // Сохраняем в файл
    m_settings->saveSettings();
}

void SettingsWindow::resetToDefaults()
{
    int ret = QMessageBox::question(this, "Сброс настроек",
                                   "Вы уверены, что хотите сбросить все настройки к значениям по умолчанию?",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_settings->resetToDefaults();
        loadSettings();
    }
}

void SettingsWindow::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsWindow::reject()
{
    QDialog::reject();
}

void SettingsWindow::onResetClicked()
{
    resetToDefaults();
}

void SettingsWindow::onBrowseOutputPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения",
                                                   m_outputPathEdit->text());
    if (!dir.isEmpty()) {
        m_outputPathEdit->setText(dir);
    }
}

void SettingsWindow::onImageQualityChanged(int value)
{
    m_qualityValueLabel->setText(QString::number(value) + "%");
}

void SettingsWindow::closeEvent(QCloseEvent *event)
{
    // Принудительно закрываем окно
    event->accept();
    QDialog::closeEvent(event);
}
