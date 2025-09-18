#include "filemanager.h"
#include "../core/settings.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

FileManager::FileManager(QObject *parent)
    : QObject(parent)
{
    Settings& settings = Settings::getInstance();
    defaultOutputPath = settings.getDefaultOutputPath();
}

FileManager::~FileManager()
{
}

QString FileManager::generateFileName(const ProfileManager::ScanProfile& profile)
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

bool FileManager::saveImage(const QImage& image, const QString& filename, int quality)
{
    if (image.isNull()) {
        emit fileSaveError("Изображение пустое");
        return false;
    }
    
    // Создаем директорию если она не существует
    QFileInfo fileInfo(filename);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit fileSaveError("Не удалось создать директорию: " + dir.absolutePath());
            return false;
        }
    }
    
    // Сохраняем изображение
    bool success = image.save(filename, nullptr, quality);
    
    if (success) {
        emit fileSaved(filename);
        qDebug() << "Image saved successfully:" << filename;
    } else {
        emit fileSaveError("Не удалось сохранить файл: " + filename);
    }
    
    return success;
}

bool FileManager::saveImageAsPDF(const QImage& image, const QString& filename, int quality, 
                                const QString& resolution, const QString& scanArea)
{
    if (image.isNull()) {
        emit fileSaveError("Изображение пустое");
        return false;
    }
    
    // Создаем директорию если она не существует
    QFileInfo fileInfo(filename);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit fileSaveError("Не удалось создать директорию: " + dir.absolutePath());
            return false;
        }
    }
    
    try {
        // Создаем PDF документ
        QPdfWriter pdfWriter(filename);
        
        // Устанавливаем размер страницы согласно настройкам профиля
        QPageSize::PageSizeId pageSize = getPageSizeFromString(scanArea);
        pdfWriter.setPageSize(QPageSize(pageSize));
        
        // Извлекаем DPI из разрешения профиля
        int dpi = getDpiFromResolution(resolution);
        pdfWriter.setResolution(dpi);
        
        QPainter painter(&pdfWriter);
        
        // Масштабируем изображение под размер страницы
        QRect pageRect = painter.viewport();
        scaleImageToPage(painter, image, pageRect);
        
        painter.end();
        
        emit fileSaved(filename);
        qDebug() << "PDF saved successfully:" << filename;
        return true;
        
    } catch (const std::exception& e) {
        emit fileSaveError("Ошибка при создании PDF: " + QString::fromStdString(e.what()));
        return false;
    }
}

QString FileManager::getDefaultOutputPath() const
{
    return defaultOutputPath;
}

void FileManager::setDefaultOutputPath(const QString& path)
{
    if (defaultOutputPath != path) {
        defaultOutputPath = path;
        emit outputPathChanged(path);
    }
}

QString FileManager::selectOutputDirectory(QWidget* parent)
{
    QString dir = QFileDialog::getExistingDirectory(parent, "Выберите папку для сохранения", defaultOutputPath);
    if (!dir.isEmpty()) {
        setDefaultOutputPath(dir);
    }
    return dir;
}

bool FileManager::fileExists(const QString& filename) const
{
    return QFileInfo::exists(filename);
}

bool FileManager::createDirectory(const QString& path) const
{
    QDir dir;
    return dir.mkpath(path);
}

QString FileManager::getFileExtension(const QString& filename) const
{
    QFileInfo fileInfo(filename);
    return fileInfo.suffix();
}

QString FileManager::getFileNameWithoutExtension(const QString& filename) const
{
    QFileInfo fileInfo(filename);
    return fileInfo.baseName();
}

QPageSize::PageSizeId FileManager::getPageSizeFromString(const QString& scanArea) const
{
    if (scanArea.contains("A4", Qt::CaseInsensitive)) {
        return QPageSize::A4;
    } else if (scanArea.contains("A3", Qt::CaseInsensitive)) {
        return QPageSize::A3;
    } else if (scanArea.contains("A5", Qt::CaseInsensitive)) {
        return QPageSize::A5;
    } else if (scanArea.contains("Letter", Qt::CaseInsensitive)) {
        return QPageSize::Letter;
    } else if (scanArea.contains("Legal", Qt::CaseInsensitive)) {
        return QPageSize::Legal;
    } else if (scanArea.contains("Tabloid", Qt::CaseInsensitive)) {
        return QPageSize::Tabloid;
    }
    
    return QPageSize::A4; // По умолчанию
}

int FileManager::getDpiFromResolution(const QString& resolution) const
{
    // Ищем DPI в строке разрешения (например "300 DPI")
    if (resolution.contains("DPI")) {
        QString dpiStr = resolution.split(" ").first();
        bool ok;
        int dpi = dpiStr.toInt(&ok);
        if (ok) {
            return dpi;
        }
    }
    
    return 300; // По умолчанию
}

void FileManager::scaleImageToPage(QPainter& painter, const QImage& image, const QRect& pageRect)
{
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
}

QImage FileManager::cropImage(const QImage& image, const QRect& cropArea) const
{
    if (image.isNull() || cropArea.isEmpty()) {
        return QImage();
    }
    
    // Проверяем, что область обрезки находится в пределах изображения
    QRect validArea = cropArea.intersected(QRect(0, 0, image.width(), image.height()));
    
    if (validArea.isEmpty()) {
        return QImage();
    }
    
    // Создаем обрезанное изображение
    return image.copy(validArea);
}

QImage FileManager::resizeImage(const QImage& image, const QSize& size) const
{
    if (image.isNull() || size.isEmpty()) {
        return QImage();
    }
    
    return image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
