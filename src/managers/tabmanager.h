#ifndef TABMANAGER_H
#define TABMANAGER_H

#include <QObject>
#include <QTabWidget>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <QMessageBox>
#include <QInputDialog>
#include <QTabBar>
#include <QLabel>
#include <vector>
#include <QString>
#include "profilemanager.h"

class TabManager : public QObject
{
    Q_OBJECT

public:
    struct TabInfo {
        QString name;
        QString fileName;
        int deviceIndex;
        std::vector<ProfileManager::ScanProfile> profiles;
    };

    explicit TabManager(QTabWidget* tabWidget, QObject *parent = nullptr);
    ~TabManager();

    // Основные методы
    void initializeTabs();
    void createNewTab(const QString& tabName);
    void deleteTab(int tabIndex);
    void setupTabContent(int tabIndex);
    void addProfileButton(int tabIndex, const ProfileManager::ScanProfile& profile);
    
    // Методы для работы с вкладками
    void loadTabs();
    void saveTabs();
    void showTabContextMenu(const QPoint& pos);
    void showTabContextMenuForProfiles(int tabIndex, const QPoint& pos);
    void createNewTabDialog();
    
    // Геттеры
    const std::vector<TabInfo>& getTabInfos() const { return tabInfos; }
    TabInfo* getTabInfo(int index);
    int getPlusTabIndex() const { return plusTabIndex_; }
    int getCurrentTabIndex() const;
    
    // Методы для работы с профилями
    void addProfileToTab(int tabIndex, const ProfileManager::ScanProfile& profile);
    void removeProfileFromTab(int tabIndex, const QString& profileName);
    void updateProfileInTab(int tabIndex, const ProfileManager::ScanProfile& profile);

signals:
    void tabChanged(int index);
    void tabCreated(const QString& tabName);
    void tabDeleted(int tabIndex);
    void profileButtonClicked(const ProfileManager::ScanProfile& profile);
    void profileContextMenuRequested(int tabIndex, const ProfileManager::ScanProfile& profile, const QPoint& pos);

private slots:
    void onTabChanged(int index);
    void onPlusTabClicked();
    void onProfileButtonClicked();
    void onProfileContextMenuRequested();

private:
    QTabWidget* tabWidget;
    std::vector<TabInfo> tabInfos;
    int contextMenuTabIndex;
    int plusTabIndex_;
    
    void addPlusTab();
    void clearTabContent(QWidget* tabWidget);
    void setupProfileButton(QPushButton* button, const ProfileManager::ScanProfile& profile);
    void connectProfileButton(QPushButton* button, const ProfileManager::ScanProfile& profile);
};

#endif // TABMANAGER_H
