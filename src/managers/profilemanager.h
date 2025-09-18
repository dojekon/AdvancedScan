#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>
#include <QImage>
#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QPageSize>
#include <QFileInfo>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <vector>
#include "../core/settings.h"

class ProfileManager : public QObject
{
    Q_OBJECT

public:
    struct ScanProfile {
        QString name;
        QString colorMode;
        QString resolution;
        QString scanArea;
        QString outputPath;
        QString filePrefix;
        QString fileFormat;
        QString outputFormat;
        int quality;
        QString buttonText;
    };

    explicit ProfileManager(QObject *parent = nullptr);
    ~ProfileManager();

    // Основные методы
    void showProfileDialog(int tabIndex, const QStringList& colorModes, 
                          const QStringList& resolutions, const QStringList& scanAreas);
    void showEditProfileDialog(int tabIndex, const ScanProfile& profile,
                              const QStringList& colorModes, 
                              const QStringList& resolutions, 
                              const QStringList& scanAreas);
    void showProfileContextMenu(int tabIndex, const ScanProfile& profile, const QPoint& pos);
    
    // Методы для работы с профилями
    QString generateFileName(const ScanProfile& profile);
    void saveImageAsPDF(const QImage& image, const QString& filename, int quality, 
                       const QString& resolution, const QString& scanArea);
    
    // Геттеры
    const std::vector<ScanProfile>& getProfiles(int tabIndex) const;
    void setProfiles(int tabIndex, const std::vector<ScanProfile>& profiles);

signals:
    void profileCreated(int tabIndex, const ScanProfile& profile);
    void profileUpdated(int tabIndex, const ScanProfile& profile);
    void profileDeleted(int tabIndex, const QString& profileName);
    void profileSelected(const ScanProfile& profile);

private slots:
    void onProfileCreated(int tabIndex, const ScanProfile& profile);
    void onProfileUpdated(int tabIndex, const ScanProfile& profile);
    void onProfileDeleted(int tabIndex, const QString& profileName);

private:
    void setupProfileDialog(QDialog& dialog, QVBoxLayout* mainLayout, 
                           const QStringList& colorModes, 
                           const QStringList& resolutions, 
                           const QStringList& scanAreas);
    void setupEditProfileDialog(QDialog& dialog, QVBoxLayout* mainLayout, 
                               const ScanProfile& profile,
                               const QStringList& colorModes, 
                               const QStringList& resolutions, 
                               const QStringList& scanAreas);
    void updatePreview(QLabel* previewLabel, QLineEdit* prefixEdit, 
                      QComboBox* formatCombo, QComboBox* outputFormatCombo);
    ScanProfile createProfileFromDialog(QDialog& dialog, 
                                       QLineEdit* nameEdit,
                                       QComboBox* colorCombo,
                                       QComboBox* resolutionCombo,
                                       QComboBox* areaCombo,
                                       QSpinBox* qualitySpin,
                                       QLineEdit* prefixEdit,
                                       QComboBox* formatCombo,
                                       QComboBox* outputFormatCombo);
    
    std::vector<std::vector<ScanProfile>> tabProfiles;
};

#endif // PROFILEMANAGER_H
