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
#include <vector>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CScanner;
class SettingsWindow;

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
    void searchScannersAsync();
    void onTabChanged(int index);
    void showTabContextMenu(const QPoint& pos);
    void deleteTab();

private:
    Ui::MainWindow *ui;
    std::vector<std::unique_ptr<CScanner>> devices;
    CScanner* activeDevice;
    int selectedDeviceIndex;
    SettingsWindow* settingsWindow;
    
    struct ScanProfile {
        QString name;
        QString colorMode;
        QString resolution;
        QString scanArea;
        QString outputPath;
        QString filePrefix;
        int quality;
        QString buttonText;
    };
    
    struct TabInfo {
        QString name;
        QString fileName;
        int deviceIndex;
        std::vector<ScanProfile> profiles;
    };
    std::vector<TabInfo> tabInfos;
    int contextMenuTabIndex;
    int plusTabIndex_;
    
    void selectDevice(int deviceIndex);
    void clearDevices();
    void loadSettings();
    void applySettings();
    void loadTabs();
    void saveTabs();
    void initializeTabs();
    void addPlusTab();
    void createNewTab(const QString& tabName);
    void createNewTabDialog();
    void setupTabContent(int tabIndex);
    void addProfileButton(int tabIndex, const ScanProfile& profile);
    void scanWithProfile(const ScanProfile& profile);
    void showProfileDialog(int tabIndex);
};
#endif // MAINWINDOW_H
