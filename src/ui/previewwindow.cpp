#include "previewwindow.h"
#include "../managers/scannermanager.h"
#include "../managers/filemanager.h"
#include "../core/cscanfront.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QMessageBox>
#include <QDebug>

PreviewImageWidget::PreviewImageWidget(QWidget* parent)
    : QLabel(parent)
    , isSelecting(false)
{
    setMinimumSize(400, 300);
    setStyleSheet("border: 1px solid #ccc;");
    setAlignment(Qt::AlignCenter);
    setScaledContents(false);
}

void PreviewImageWidget::setImage(const QImage& image)
{
    originalImage = image;
    
    // Масштабируем изображение для отображения
    QSize widgetSize = size();
    QSize imageSize = image.size();
    
    // Вычисляем масштаб для вписывания изображения в виджет
    qreal scaleX = static_cast<qreal>(widgetSize.width()) / imageSize.width();
    qreal scaleY = static_cast<qreal>(widgetSize.height()) / imageSize.height();
    qreal scale = qMin(scaleX, scaleY);
    
    QSize scaledSize = imageSize * scale;
    scaledImageRect = QRect((widgetSize.width() - scaledSize.width()) / 2,
                           (widgetSize.height() - scaledSize.height()) / 2,
                           scaledSize.width(),
                           scaledSize.height());
    
    // Создаем масштабированное изображение для отображения
    QPixmap scaledPixmap = QPixmap::fromImage(image.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setPixmap(scaledPixmap);
    
    // Сбрасываем выделение
    selectedArea = QRect();
    update();
}


void PreviewImageWidget::setSelectedArea(const QRect& area)
{
    selectedArea = area;
    update();
}

void PreviewImageWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !originalImage.isNull()) {
        isSelecting = true;
        selectionStart = event->pos();
        selectedArea = QRect();
        update();
    }
}

void PreviewImageWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (isSelecting && !originalImage.isNull()) {
        QPoint currentPos = event->pos();
        selectedArea = QRect(selectionStart, currentPos).normalized();
        update();
    }
}

void PreviewImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isSelecting) {
        isSelecting = false;
        
        if (!selectedArea.isEmpty()) {
            // Преобразуем координаты виджета в координаты изображения
            QPoint topLeft = mapToImage(selectedArea.topLeft());
            QPoint bottomRight = mapToImage(selectedArea.bottomRight());
            QRect imageArea = QRect(topLeft, bottomRight);
            emit areaSelected(imageArea);
        }
    }
}

void PreviewImageWidget::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);
    
    if (!selectedArea.isEmpty()) {
        QPainter painter(this);
        painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
        painter.setBrush(QBrush(QColor(255, 0, 0, 50)));
        painter.drawRect(selectedArea);
    }
}

QPoint PreviewImageWidget::mapToImage(const QPoint& widgetPos) const
{
    if (originalImage.isNull() || scaledImageRect.isEmpty()) {
        return QPoint();
    }
    
    // Проверяем, что точка находится в области изображения
    if (!scaledImageRect.contains(widgetPos)) {
        return QPoint();
    }
    
    // Вычисляем масштаб
    qreal scaleX = static_cast<qreal>(originalImage.width()) / scaledImageRect.width();
    qreal scaleY = static_cast<qreal>(originalImage.height()) / scaledImageRect.height();
    
    // Преобразуем координаты
    QPoint imagePos = QPoint(
        (widgetPos.x() - scaledImageRect.x()) * scaleX,
        (widgetPos.y() - scaledImageRect.y()) * scaleY
    );
    
    return imagePos;
}

QRect PreviewImageWidget::mapFromImage(const QRect& imageRect) const
{
    if (originalImage.isNull() || scaledImageRect.isEmpty()) {
        return QRect();
    }
    
    // Вычисляем масштаб
    qreal scaleX = static_cast<qreal>(scaledImageRect.width()) / originalImage.width();
    qreal scaleY = static_cast<qreal>(scaledImageRect.height()) / originalImage.height();
    
    // Преобразуем координаты
    QRect widgetRect = QRect(
        scaledImageRect.x() + imageRect.x() * scaleX,
        scaledImageRect.y() + imageRect.y() * scaleY,
        imageRect.width() * scaleX,
        imageRect.height() * scaleY
    );
    
    return widgetRect;
}

PreviewWindow::PreviewWindow(const ProfileManager::ScanProfile& profile, 
                            ScannerManager* scannerManager, 
                            QWidget* parent)
    : QDialog(parent)
    , profile(profile)
    , scannerManager(scannerManager)
    , hasPreview(false)
{
    setWindowTitle("Предпросмотр области сканирования");
    setModal(true);
    resize(800, 600);
    
    setupUI();
    updateButtons();
}

PreviewWindow::~PreviewWindow()
{
}

void PreviewWindow::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Статус
    statusLabel = new QLabel("Нажмите 'Сканировать' для получения предпросмотра");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);
    
    // Область изображения
    scrollArea = new QScrollArea();
    imageWidget = new PreviewImageWidget();
    scrollArea->setWidget(imageWidget);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);
    
    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    scanButton = new QPushButton("Сканировать");
    applyButton = new QPushButton("Применить");
    cancelButton = new QPushButton("Отмена");
    
    buttonLayout->addWidget(scanButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Подключаем сигналы
    connect(scanButton, &QPushButton::clicked, this, &PreviewWindow::onScanClicked);
    connect(applyButton, &QPushButton::clicked, this, &PreviewWindow::onApplyClicked);
    connect(cancelButton, &QPushButton::clicked, this, &PreviewWindow::onCancelClicked);
    connect(imageWidget, &PreviewImageWidget::areaSelected, this, &PreviewWindow::onAreaSelected);
}

void PreviewWindow::updateButtons()
{
    scanButton->setEnabled(!hasPreview);
    applyButton->setEnabled(hasPreview && !selectedArea.isEmpty());
}

QRect PreviewWindow::getSelectedArea() const
{
    qDebug() << "PreviewWindow: getSelectedArea() returning:" << selectedArea;
    return selectedArea;
}

void PreviewWindow::onScanClicked()
{
    if (!scannerManager || !scannerManager->getActiveDevice()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран сканер");
        return;
    }
    
    statusLabel->setText("Сканирование...");
    scanButton->setEnabled(false);
    
    try {
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
        
        // Для предпросмотра всегда используем максимальную область, чтобы получить полное изображение
        // Находим индекс "Maximum Area" или используем первый доступный
        int maxAreaIndex = scanAreas.indexOf("Maximum Area");
        if (maxAreaIndex < 0) {
            maxAreaIndex = 0; // Используем первый доступный
        }
        scannerManager->getActiveDevice()->setSelectedScanArea(maxAreaIndex);
        
        // Сбрасываем пользовательскую область для предпросмотра
        scannerManager->getActiveDevice()->resetCustomScanArea();
        
        qDebug() << "PreviewWindow: Using maximum area for preview scan";
        
        // Сканируем изображение
        QImage scannedImage = CScanFront::scanImage(scannerManager->getActiveDevice());
        
        if (!scannedImage.isNull()) {
            imageWidget->setImage(scannedImage);
            hasPreview = true;
            statusLabel->setText("Выделите область для сканирования");
        } else {
            statusLabel->setText("Ошибка сканирования");
        }
        
    } catch(const std::exception& e) {
        QMessageBox::critical(this, "Ошибка сканирования", 
                            "Ошибка сканирования: " + QString::fromStdString(e.what()));
        statusLabel->setText("Ошибка сканирования");
    } catch(...) {
        QMessageBox::critical(this, "Ошибка сканирования", 
                            "Неизвестная ошибка сканирования");
        statusLabel->setText("Ошибка сканирования");
    }
    
    scanButton->setEnabled(true);
    updateButtons();
}

void PreviewWindow::onAreaSelected(const QRect& area)
{
    selectedArea = area;
    qDebug() << "PreviewWindow: Area selected:" << area;
    statusLabel->setText(QString("Выделена область: %1x%2 в позиции (%3, %4)")
                        .arg(area.width())
                        .arg(area.height())
                        .arg(area.x())
                        .arg(area.y()));
    updateButtons();
}

void PreviewWindow::onApplyClicked()
{
    if (!selectedArea.isEmpty()) {
        emit areaSelected(selectedArea);
        accept();
    }
}

void PreviewWindow::onCancelClicked()
{
    reject();
}
