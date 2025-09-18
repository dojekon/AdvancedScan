#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <QTabWidget>
#include <QTabBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <vector>
#include <memory>
#include "../managers/profilemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CScanner;
class SettingsWindow;
class ScannerManager;
class TabManager;
class FileManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_pushButton_2_clicked();
    void onTabChanged(int index);
    void onProfileButtonClicked(const ProfileManager::ScanProfile& profile);
    void onProfileContextMenuRequested(int tabIndex, const ProfileManager::ScanProfile& profile, const QPoint& pos);
    void onScannersFound();
    void onScannerSelected();
    void onFileSaved(const QString& filename);
    void onFileSaveError(const QString& error);

private:
    Ui::MainWindow *ui;
    SettingsWindow* settingsWindow;
    
    // Менеджеры
    ScannerManager* scannerManager;
    ProfileManager* profileManager;
    TabManager* tabManager;
    FileManager* fileManager;
    
    void setupConnections();
    void loadSettings();
    void applySettings();
    void scanWithProfile(const ProfileManager::ScanProfile& profile);
    void updateScannerComboBox();
    void updateStatusBar(const QString& message);
};

#endif // MAINWINDOW_H
