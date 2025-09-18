#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QSlider>
#include <QCloseEvent>
#include "../core/settings.h"

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

private slots:
    void onResetClicked();
    void onBrowseOutputPath();
    void onImageQualityChanged(int value);
    
protected:
    void closeEvent(QCloseEvent *event) override;
    void accept() override;
    void reject() override;

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void resetToDefaults();
    
    // UI элементы
    QTabWidget *m_tabWidget;
    
    // Вкладка "Сканирование"
    QWidget *m_scanningTab;
    QComboBox *m_colorModeCombo;
    QComboBox *m_resolutionCombo;
    QComboBox *m_scanAreaCombo;
    QLineEdit *m_outputPathEdit;
    QLineEdit *m_filePrefixEdit;
    QPushButton *m_browseButton;
    
    // Вкладка "Приложение"
    QWidget *m_applicationTab;
    QCheckBox *m_autoFindScannersCheck;
    QCheckBox *m_showNotificationsCheck;
    QSpinBox *m_scanTimeoutSpin;
    
    // Вкладка "Качество"
    QWidget *m_qualityTab;
    QSlider *m_imageQualitySlider;
    QLabel *m_qualityValueLabel;
    QCheckBox *m_autoCropCheck;
    
    // Кнопки
    QPushButton *m_resetButton;
    
    Settings *m_settings;
};

#endif // SETTINGSWINDOW_H
