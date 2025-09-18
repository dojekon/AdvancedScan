#include "profilemanager.h"
#include "../core/settings.h"
#include "../ui/previewwindow.h"
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QCursor>

ProfileManager::ProfileManager(QObject *parent)
    : QObject(parent)
    , scannerManager(nullptr)
{
}

ProfileManager::~ProfileManager()
{
}

void ProfileManager::showProfileDialog(int tabIndex, const QStringList& colorModes, 
                                      const QStringList& resolutions, const QStringList& scanAreas)
{
    QDialog dialog;
    dialog.setWindowTitle("Создать профиль сканирования");
    dialog.setModal(true);
    dialog.resize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    setupProfileDialog(dialog, mainLayout, colorModes, resolutions, scanAreas, nullptr);

    if (dialog.exec() == QDialog::Accepted) {
        // Получаем данные из диалога
        QLineEdit* nameEdit = dialog.findChild<QLineEdit*>("nameEdit");
        QComboBox* colorCombo = dialog.findChild<QComboBox*>("colorCombo");
        QComboBox* resolutionCombo = dialog.findChild<QComboBox*>("resolutionCombo");
        QComboBox* areaCombo = dialog.findChild<QComboBox*>("areaCombo");
        QSpinBox* qualitySpin = dialog.findChild<QSpinBox*>("qualitySpin");
        QLineEdit* prefixEdit = dialog.findChild<QLineEdit*>("prefixEdit");
        QComboBox* formatCombo = dialog.findChild<QComboBox*>("formatCombo");
        QComboBox* outputFormatCombo = dialog.findChild<QComboBox*>("outputFormatCombo");

        if (nameEdit && !nameEdit->text().isEmpty()) {
            ScanProfile newProfile = createProfileFromDialog(dialog, nameEdit, colorCombo, 
                                                           resolutionCombo, areaCombo, qualitySpin,
                                                           prefixEdit, formatCombo, outputFormatCombo);
            emit profileCreated(tabIndex, newProfile);
        }
    }
}

void ProfileManager::showEditProfileDialog(int tabIndex, const ScanProfile& profile,
                                          const QStringList& colorModes, 
                                          const QStringList& resolutions, 
                                          const QStringList& scanAreas)
{
    QDialog dialog;
    dialog.setWindowTitle("Редактировать профиль");
    dialog.setModal(true);
    dialog.resize(500, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    setupEditProfileDialog(dialog, mainLayout, profile, colorModes, resolutions, scanAreas);

    if (dialog.exec() == QDialog::Accepted) {
        // Получаем данные из диалога
        QLineEdit* nameEdit = dialog.findChild<QLineEdit*>("nameEdit");
        QComboBox* colorCombo = dialog.findChild<QComboBox*>("colorCombo");
        QComboBox* resolutionCombo = dialog.findChild<QComboBox*>("resolutionCombo");
        QComboBox* areaCombo = dialog.findChild<QComboBox*>("areaCombo");
        QSpinBox* qualitySpin = dialog.findChild<QSpinBox*>("qualitySpin");
        QLineEdit* prefixEdit = dialog.findChild<QLineEdit*>("prefixEdit");
        QComboBox* formatCombo = dialog.findChild<QComboBox*>("formatCombo");
        QComboBox* outputFormatCombo = dialog.findChild<QComboBox*>("outputFormatCombo");

        if (nameEdit) {
            ScanProfile updatedProfile = createProfileFromDialog(dialog, nameEdit, colorCombo, 
                                                               resolutionCombo, areaCombo, qualitySpin,
                                                               prefixEdit, formatCombo, outputFormatCombo);
            updatedProfile.name = nameEdit->text();
            emit profileUpdated(tabIndex, updatedProfile);
        }
    }
}

void ProfileManager::showProfileContextMenu(int tabIndex, const ScanProfile& profile, const QPoint& pos)
{
    QMenu contextMenu;
    
    QAction* editAction = contextMenu.addAction("Редактировать профиль");
    QAction* deleteAction = contextMenu.addAction("Удалить профиль");
    
    QAction* selectedAction = contextMenu.exec(QCursor::pos());
    
    if (selectedAction == editAction) {
        // Получаем списки поддерживаемых режимов (можно передать как параметр)
        QStringList colorModes = {"Цветной", "Черно-белый", "Оттенки серого"};
        QStringList resolutions = {"75 DPI", "150 DPI", "300 DPI", "600 DPI"};
        QStringList scanAreas = {"A4", "A3", "A5", "Letter", "Legal", "Tabloid"};
        
        showEditProfileDialog(tabIndex, profile, colorModes, resolutions, scanAreas);
    } else if (selectedAction == deleteAction) {
        emit profileDeleted(tabIndex, profile.name);
    }
}

QString ProfileManager::generateFileName(const ScanProfile& profile)
{
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
    
    // Добавляем расширение файла согласно выбранному формату
    QString extension = "." + profile.outputFormat.toLower();
    return profile.outputPath + "/" + filename + extension;
}

void ProfileManager::saveImageAsPDF(const QImage& image, const QString& filename, int quality, 
                                   const QString& resolution, const QString& scanArea)
{
    // Создаем PDF документ
    QPdfWriter pdfWriter(filename);
    
    // Устанавливаем размер страницы согласно настройкам профиля
    QPageSize::PageSizeId pageSize = QPageSize::A4; // По умолчанию
    
    if (scanArea.contains("A4", Qt::CaseInsensitive)) {
        pageSize = QPageSize::A4;
    } else if (scanArea.contains("A3", Qt::CaseInsensitive)) {
        pageSize = QPageSize::A3;
    } else if (scanArea.contains("A5", Qt::CaseInsensitive)) {
        pageSize = QPageSize::A5;
    } else if (scanArea.contains("Letter", Qt::CaseInsensitive)) {
        pageSize = QPageSize::Letter;
    } else if (scanArea.contains("Legal", Qt::CaseInsensitive)) {
        pageSize = QPageSize::Legal;
    } else if (scanArea.contains("Tabloid", Qt::CaseInsensitive)) {
        pageSize = QPageSize::Tabloid;
    }
    
    pdfWriter.setPageSize(QPageSize(pageSize));
    
    // Извлекаем DPI из разрешения профиля
    int dpi = 300; // По умолчанию
    
    // Ищем DPI в строке разрешения (например "300 DPI")
    if (resolution.contains("DPI")) {
        QString dpiStr = resolution.split(" ").first();
        bool ok;
        int resDpi = dpiStr.toInt(&ok);
        if (ok) {
            dpi = resDpi;
        }
    }
    pdfWriter.setResolution(dpi);
    
    QPainter painter(&pdfWriter);
    
    // Масштабируем изображение под размер страницы
    QRect pageRect = painter.viewport();
    QSize imageSize = image.size();
    
    // Вычисляем масштаб для вписывания изображения в страницу
    qreal scaleX = static_cast<qreal>(pageRect.width()) / imageSize.width();
    qreal scaleY = static_cast<qreal>(pageRect.height()) / imageSize.height();
    qreal scale = qMin(scaleX, scaleY);
    
    QSize scaledSize = imageSize * scale;
    QRect imageRect((pageRect.width() - scaledSize.width()) / 2,
                    (pageRect.height() - scaledSize.height()) / 2,
                    scaledSize.width(),
                    scaledSize.height());
    
    // Рисуем изображение
    painter.drawImage(imageRect, image);
    painter.end();
}

const std::vector<ProfileManager::ScanProfile>& ProfileManager::getProfiles(int tabIndex) const
{
    if (tabIndex >= 0 && tabIndex < static_cast<int>(tabProfiles.size())) {
        return tabProfiles[tabIndex];
    }
    static std::vector<ScanProfile> empty;
    return empty;
}

void ProfileManager::setProfiles(int tabIndex, const std::vector<ScanProfile>& profiles)
{
    if (tabIndex >= 0) {
        if (tabIndex >= static_cast<int>(tabProfiles.size())) {
            tabProfiles.resize(tabIndex + 1);
        }
        tabProfiles[tabIndex] = profiles;
    }
}

void ProfileManager::setupProfileDialog(QDialog& dialog, QVBoxLayout* mainLayout,
                              const QStringList& colorModes, 
                              const QStringList& resolutions, 
                              const QStringList& scanAreas,
                              ScanProfile* tempProfile)
{
    // Поле для имени профиля
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Имя профиля:");
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setObjectName("nameEdit");
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
    colorCombo->setObjectName("colorCombo");
    colorCombo->addItems(colorModes);
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorCombo);
    settingsLayout->addLayout(colorLayout);

    // Разрешение
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    QLabel* resolutionLabel = new QLabel("Разрешение:");
    QComboBox* resolutionCombo = new QComboBox();
    resolutionCombo->setObjectName("resolutionCombo");
    resolutionCombo->addItems(resolutions);
    resolutionLayout->addWidget(resolutionLabel);
    resolutionLayout->addWidget(resolutionCombo);
    settingsLayout->addLayout(resolutionLayout);

    // Область сканирования
    QHBoxLayout* areaLayout = new QHBoxLayout();
    QLabel* areaLabel = new QLabel("Область сканирования:");
    QComboBox* areaCombo = new QComboBox();
    areaCombo->setObjectName("areaCombo");
    areaCombo->addItems(scanAreas);
    areaLayout->addWidget(areaLabel);
    areaLayout->addWidget(areaCombo);
    settingsLayout->addLayout(areaLayout);

    // Качество изображения
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    QLabel* qualityLabel = new QLabel("Качество:");
    QSpinBox* qualitySpin = new QSpinBox();
    qualitySpin->setObjectName("qualitySpin");
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
    prefixEdit->setObjectName("prefixEdit");
    prefixEdit->setPlaceholderText("Например: Документ");
    prefixLayout->addWidget(prefixLabel);
    prefixLayout->addWidget(prefixEdit);
    fileLayout->addLayout(prefixLayout);

    // Формат названия файла
    QHBoxLayout* formatLayout = new QHBoxLayout();
    QLabel* formatLabel = new QLabel("Формат названия:");
    QComboBox* formatCombo = new QComboBox();
    formatCombo->setObjectName("formatCombo");
    formatCombo->addItem("Префикс_ДатаВремя", "PREFIX_DATETIME");
    formatCombo->addItem("Префикс_Дата", "PREFIX_DATE");
    formatCombo->addItem("Префикс_Время", "PREFIX_TIME");
    formatCombo->addItem("Префикс_Номер", "PREFIX_NUMBER");
    formatCombo->addItem("Только префикс", "PREFIX_ONLY");
    formatCombo->addItem("Только дата и время", "DATETIME_ONLY");
    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(formatCombo);
    fileLayout->addLayout(formatLayout);

    // Формат выходного файла
    QHBoxLayout* outputFormatLayout = new QHBoxLayout();
    QLabel* outputFormatLabel = new QLabel("Формат файла:");
    QComboBox* outputFormatCombo = new QComboBox();
    outputFormatCombo->setObjectName("outputFormatCombo");
    outputFormatCombo->addItem("PNG", "PNG");
    outputFormatCombo->addItem("JPEG", "JPEG");
    outputFormatCombo->addItem("PDF", "PDF");
    outputFormatCombo->addItem("TIFF", "TIFF");
    outputFormatCombo->addItem("BMP", "BMP");
    outputFormatLayout->addWidget(outputFormatLabel);
    outputFormatLayout->addWidget(outputFormatCombo);
    fileLayout->addLayout(outputFormatLayout);

    // Предварительный просмотр названия файла
    QHBoxLayout* previewLayout = new QHBoxLayout();
    QLabel* previewLabel = new QLabel("Предварительный просмотр:");
    QLabel* previewText = new QLabel("Документ_2024-01-15_14-30-25");
    previewText->setObjectName("previewText");
    previewText->setStyleSheet("color: #666; font-style: italic;");
    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(previewText);
    fileLayout->addLayout(previewLayout);

    // Функция обновления предварительного просмотра
    auto updatePreview = [=]() {
        QString prefix = prefixEdit->text().isEmpty() ? "Документ" : prefixEdit->text();
        QString format = formatCombo->currentData().toString();
        QString outputFormat = outputFormatCombo->currentData().toString();
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
        
        // Добавляем расширение файла
        preview += "." + outputFormat.toLower();
        
        previewText->setText(preview);
    };

    connect(prefixEdit, &QLineEdit::textChanged, updatePreview);
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updatePreview);
    connect(outputFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updatePreview);

    mainLayout->addWidget(fileGroup);

    // Кнопки OK/Cancel/Preview
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* previewButton = new QPushButton("Предпросмотр");
    QPushButton* okButton = new QPushButton("Создать");
    QPushButton* cancelButton = new QPushButton("Отмена");

    connect(previewButton, &QPushButton::clicked, [this, &dialog, nameEdit, colorCombo, resolutionCombo, areaCombo, qualitySpin, prefixEdit, formatCombo, outputFormatCombo, tempProfile]() {
        // Создаем временный профиль для предпросмотра
        ProfileManager::ScanProfile previewProfile;
        previewProfile.name = nameEdit->text().isEmpty() ? "Предпросмотр" : nameEdit->text();
        previewProfile.colorMode = colorCombo->currentText();
        previewProfile.resolution = resolutionCombo->currentText();
        previewProfile.scanArea = areaCombo->currentText();
        previewProfile.quality = qualitySpin->value();
        previewProfile.filePrefix = prefixEdit->text().isEmpty() ? "Документ" : prefixEdit->text();
        previewProfile.fileFormat = formatCombo->currentData().toString();
        previewProfile.outputFormat = outputFormatCombo->currentData().toString();
        previewProfile.outputPath = Settings::getInstance().getDefaultOutputPath();
        
        // Показываем диалог предпросмотра и получаем обновленный профиль
        if (scannerManager) {
            ScanProfile updatedProfile = showPreviewDialogWithResult(previewProfile, scannerManager, &dialog);
            if (tempProfile) {
                *tempProfile = updatedProfile;
                qDebug() << "ProfileManager: After preview, tempProfile->useCustomArea:" << tempProfile->useCustomArea;
                qDebug() << "ProfileManager: After preview, tempProfile->customArea:" << tempProfile->customArea;
            }
        } else {
            showPreviewDialog(previewProfile, &dialog);
        }
    });
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    buttonLayout->addWidget(previewButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
}

void ProfileManager::setupEditProfileDialog(QDialog& dialog, QVBoxLayout* mainLayout, 
                                           const ScanProfile& profile,
                                           const QStringList& colorModes, 
                                           const QStringList& resolutions, 
                                           const QStringList& scanAreas)
{
    // Название профиля
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Название профиля:");
    QLineEdit* nameEdit = new QLineEdit(profile.name);
    nameEdit->setObjectName("nameEdit");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    mainLayout->addLayout(nameLayout);

    // Режим цвета
    QGroupBox* colorGroup = new QGroupBox("Режим цвета");
    QVBoxLayout* colorLayout = new QVBoxLayout(colorGroup);
    QComboBox* colorCombo = new QComboBox();
    colorCombo->setObjectName("colorCombo");
    colorCombo->addItems(colorModes);
    colorCombo->setCurrentText(profile.colorMode);
    colorLayout->addWidget(colorCombo);
    mainLayout->addWidget(colorGroup);

    // Разрешение
    QGroupBox* resolutionGroup = new QGroupBox("Разрешение");
    QVBoxLayout* resolutionLayout = new QVBoxLayout(resolutionGroup);
    QComboBox* resolutionCombo = new QComboBox();
    resolutionCombo->setObjectName("resolutionCombo");
    resolutionCombo->addItems(resolutions);
    resolutionCombo->setCurrentText(profile.resolution);
    resolutionLayout->addWidget(resolutionCombo);
    mainLayout->addWidget(resolutionGroup);

    // Область сканирования
    QGroupBox* areaGroup = new QGroupBox("Область сканирования");
    QVBoxLayout* areaLayout = new QVBoxLayout(areaGroup);
    QComboBox* areaCombo = new QComboBox();
    areaCombo->setObjectName("areaCombo");
    areaCombo->addItems(scanAreas);
    areaCombo->setCurrentText(profile.scanArea);
    areaLayout->addWidget(areaCombo);
    mainLayout->addWidget(areaGroup);

    // Путь сохранения
    QGroupBox* pathGroup = new QGroupBox("Путь сохранения");
    QVBoxLayout* pathLayout = new QVBoxLayout(pathGroup);
    QHBoxLayout* pathButtonLayout = new QHBoxLayout();
    QLineEdit* pathEdit = new QLineEdit(profile.outputPath);
    pathEdit->setObjectName("pathEdit");
    QPushButton* pathButton = new QPushButton("Выбрать");
    pathButtonLayout->addWidget(pathEdit);
    pathButtonLayout->addWidget(pathButton);
    pathLayout->addLayout(pathButtonLayout);
    mainLayout->addWidget(pathGroup);

    connect(pathButton, &QPushButton::clicked, [pathEdit]() {
        QString dir = QFileDialog::getExistingDirectory(nullptr, "Выберите папку для сохранения");
        if (!dir.isEmpty()) {
            pathEdit->setText(dir);
        }
    });

    // Префикс файла
    QGroupBox* prefixGroup = new QGroupBox("Префикс файла");
    QVBoxLayout* prefixLayout = new QVBoxLayout(prefixGroup);
    QLineEdit* prefixEdit = new QLineEdit(profile.filePrefix);
    prefixEdit->setObjectName("prefixEdit");
    prefixLayout->addWidget(prefixEdit);
    mainLayout->addWidget(prefixGroup);

    // Формат имени файла
    QGroupBox* formatGroup = new QGroupBox("Формат имени файла");
    QVBoxLayout* formatLayout = new QVBoxLayout(formatGroup);
    QComboBox* formatCombo = new QComboBox();
    formatCombo->setObjectName("formatCombo");
    formatCombo->addItems({"PREFIX_DATETIME", "PREFIX_DATE", "PREFIX_TIME", "PREFIX_NUMBER", "PREFIX_ONLY"});
    formatCombo->setCurrentText(profile.fileFormat);
    
    QLabel* previewLabel = new QLabel("Предварительный просмотр:");
    QLabel* previewText = new QLabel();
    previewText->setObjectName("previewText");
    previewText->setStyleSheet("color: #666; font-style: italic;");
    
    formatLayout->addWidget(formatCombo);
    formatLayout->addWidget(previewLabel);
    formatLayout->addWidget(previewText);
    mainLayout->addWidget(formatGroup);

    // Формат выходного файла
    QGroupBox* outputGroup = new QGroupBox("Формат выходного файла");
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
    QComboBox* outputCombo = new QComboBox();
    outputCombo->setObjectName("outputFormatCombo");
    outputCombo->addItems({"PNG", "JPEG", "PDF", "TIFF", "BMP"});
    outputCombo->setCurrentText(profile.outputFormat);
    outputLayout->addWidget(outputCombo);
    mainLayout->addWidget(outputGroup);

    // Качество
    QGroupBox* qualityGroup = new QGroupBox("Качество");
    QVBoxLayout* qualityLayout = new QVBoxLayout(qualityGroup);
    QSpinBox* qualitySpin = new QSpinBox();
    qualitySpin->setObjectName("qualitySpin");
    qualitySpin->setRange(1, 100);
    qualitySpin->setValue(profile.quality);
    qualityLayout->addWidget(qualitySpin);
    mainLayout->addWidget(qualityGroup);

    // Обновление предварительного просмотра
    auto updatePreview = [=]() {
        QString prefix = prefixEdit->text();
        QString format = formatCombo->currentText();
        
        // Создаем временный профиль для генерации предварительного просмотра
        ProfileManager::ScanProfile tempProfile;
        tempProfile.filePrefix = prefix;
        tempProfile.fileFormat = format;
        tempProfile.outputPath = "/tmp"; // Временный путь для предварительного просмотра
        
        QString preview = generateFileName(tempProfile);
        // Убираем путь и расширение для предварительного просмотра
        QFileInfo fileInfo(preview);
        previewText->setText(fileInfo.baseName());
    };

    connect(prefixEdit, &QLineEdit::textChanged, updatePreview);
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updatePreview);
    updatePreview();

    // Кнопки OK/Cancel
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Отмена");

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
}

void ProfileManager::updatePreview(QLabel* previewLabel, QLineEdit* prefixEdit, 
                                  QComboBox* formatCombo, QComboBox* outputFormatCombo)
{
    QString prefix = prefixEdit->text().isEmpty() ? "Документ" : prefixEdit->text();
    QString format = formatCombo->currentData().toString();
    QString outputFormat = outputFormatCombo->currentData().toString();
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
    
    // Добавляем расширение файла
    preview += "." + outputFormat.toLower();
    
    previewLabel->setText(preview);
}

ProfileManager::ScanProfile ProfileManager::createProfileFromDialog(QDialog& dialog, 
                                                                   QLineEdit* nameEdit,
                                                                   QComboBox* colorCombo,
                                                                   QComboBox* resolutionCombo,
                                                                   QComboBox* areaCombo,
                                                                   QSpinBox* qualitySpin,
                                                                   QLineEdit* prefixEdit,
                                                                   QComboBox* formatCombo,
                                                                   QComboBox* outputFormatCombo)
{
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
    newProfile.outputFormat = outputFormatCombo->currentData().toString();

    Settings& settings = Settings::getInstance();
    newProfile.outputPath = settings.getDefaultOutputPath();
    
    // Инициализируем поля для пользовательской области сканирования
    newProfile.useCustomArea = false;
    newProfile.customArea = QRect();
    newProfile.previewArea = QRect();
    
    return newProfile;
}

void ProfileManager::onProfileCreated(int tabIndex, const ScanProfile& profile)
{
    // Реализация будет добавлена позже
}

void ProfileManager::onProfileUpdated(int tabIndex, const ScanProfile& profile)
{
    // Реализация будет добавлена позже
}

void ProfileManager::onProfileDeleted(int tabIndex, const QString& profileName)
{
    // Реализация будет добавлена позже
}

void ProfileManager::showPreviewDialog(const ScanProfile& profile, QWidget* parent)
{
    PreviewWindow dialog(profile, nullptr, parent);
    
    if (dialog.exec() == QDialog::Accepted) {
        QRect selectedArea = dialog.getSelectedArea();
        if (!selectedArea.isEmpty()) {
            // Создаем копию профиля с обновленной областью
            ScanProfile updatedProfile = profile;
            updatedProfile.useCustomArea = true;
            updatedProfile.customArea = selectedArea;
            
            emit profileUpdated(-1, updatedProfile); // -1 означает обновление текущего профиля
        }
    }
}

void ProfileManager::showPreviewDialog(const ScanProfile& profile, ScannerManager* scannerManager, QWidget* parent)
{
    PreviewWindow dialog(profile, scannerManager, parent);
    
    if (dialog.exec() == QDialog::Accepted) {
        QRect selectedArea = dialog.getSelectedArea();
        if (!selectedArea.isEmpty()) {
            // Создаем копию профиля с обновленной областью
            ScanProfile updatedProfile = profile;
            updatedProfile.useCustomArea = true;
            updatedProfile.customArea = selectedArea;
            
            emit profileUpdated(-1, updatedProfile); // -1 означает обновление текущего профиля
        }
    }
}

ProfileManager::ScanProfile ProfileManager::showPreviewDialogWithResult(const ScanProfile& profile, ScannerManager* scannerManager, QWidget* parent)
{
    PreviewWindow dialog(profile, scannerManager, parent);
    
    if (dialog.exec() == QDialog::Accepted) {
        QRect selectedArea = dialog.getSelectedArea();
        if (!selectedArea.isEmpty()) {
            // Создаем копию профиля с обновленной областью
            ScanProfile updatedProfile = profile;
            updatedProfile.useCustomArea = true;
            updatedProfile.customArea = selectedArea;
            return updatedProfile;
        }
    }
    
    return profile; // Возвращаем исходный профиль, если ничего не выбрано
}

ProfileManager::ScanProfile ProfileManager::showProfileDialogWithResult(int tabIndex, const QStringList& colorModes, 
                          const QStringList& resolutions, const QStringList& scanAreas)
{
    QDialog dialog;
    dialog.setWindowTitle("Создать профиль");
    dialog.setModal(true);
    dialog.resize(400, 500);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    
    // Создаем временный профиль
    ScanProfile tempProfile;
    tempProfile.useCustomArea = false;
    tempProfile.customArea = QRect();
    
    setupProfileDialog(dialog, mainLayout, colorModes, resolutions, scanAreas, &tempProfile);
    
    // Показываем диалог и получаем результат
    if (dialog.exec() == QDialog::Accepted) {
        // Создаем профиль из диалога
        QLineEdit* nameEdit = dialog.findChild<QLineEdit*>("nameEdit");
        QComboBox* colorCombo = dialog.findChild<QComboBox*>("colorCombo");
        QComboBox* resolutionCombo = dialog.findChild<QComboBox*>("resolutionCombo");
        QComboBox* areaCombo = dialog.findChild<QComboBox*>("areaCombo");
        QSpinBox* qualitySpin = dialog.findChild<QSpinBox*>("qualitySpin");
        QLineEdit* prefixEdit = dialog.findChild<QLineEdit*>("prefixEdit");
        QComboBox* formatCombo = dialog.findChild<QComboBox*>("formatCombo");
        QComboBox* outputFormatCombo = dialog.findChild<QComboBox*>("outputFormatCombo");
        
        if (nameEdit && !nameEdit->text().isEmpty()) {
            ScanProfile newProfile = createProfileFromDialog(dialog, nameEdit, colorCombo, 
                                                           resolutionCombo, areaCombo, qualitySpin,
                                                           prefixEdit, formatCombo, outputFormatCombo);
            
            // Если есть пользовательская область, добавляем её
            if (tempProfile.useCustomArea && !tempProfile.customArea.isEmpty()) {
                newProfile.useCustomArea = true;
                newProfile.customArea = tempProfile.customArea;
                qDebug() << "ProfileManager: Added custom area to profile:" << newProfile.customArea;
            }
            
            return newProfile;
        }
    }
    
    return tempProfile; // Возвращаем пустой профиль, если диалог отменен
}

void ProfileManager::updateProfileWithCustomArea(ScanProfile& profile, const QRect& customArea)
{
    profile.useCustomArea = true;
    profile.customArea = customArea;
}

void ProfileManager::setScannerManager(ScannerManager* scannerManager)
{
    this->scannerManager = scannerManager;
}
