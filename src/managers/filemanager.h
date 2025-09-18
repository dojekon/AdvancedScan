#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QPageSize>
#include <QFileInfo>
#include "profilemanager.h"

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);
    ~FileManager();

    // Основные методы
    QString generateFileName(const ProfileManager::ScanProfile& profile);
    bool saveImage(const QImage& image, const QString& filename, int quality);
    bool saveImageAsPDF(const QImage& image, const QString& filename, int quality, 
                       const QString& resolution, const QString& scanArea);
    
    // Методы для работы с путями
    QString getDefaultOutputPath() const;
    void setDefaultOutputPath(const QString& path);
    QString selectOutputDirectory(QWidget* parent = nullptr);
    
    // Методы для работы с файлами
    bool fileExists(const QString& filename) const;
    bool createDirectory(const QString& path) const;
    QString getFileExtension(const QString& filename) const;
    QString getFileNameWithoutExtension(const QString& filename) const;

signals:
    void fileSaved(const QString& filename);
    void fileSaveError(const QString& error);
    void outputPathChanged(const QString& path);

private:
    QString defaultOutputPath;
    
    QPageSize::PageSizeId getPageSizeFromString(const QString& scanArea) const;
    int getDpiFromResolution(const QString& resolution) const;
    void scaleImageToPage(QPainter& painter, const QImage& image, const QRect& pageRect);
};

#endif // FILEMANAGER_H

