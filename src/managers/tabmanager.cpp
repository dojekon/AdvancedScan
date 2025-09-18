#include "tabmanager.h"
#include "../core/settings.h"
#include <QDebug>
#include <QTabBar>
#include <QLabel>

TabManager::TabManager(QTabWidget* tabWidget, QObject *parent)
    : QObject(parent)
    , tabWidget(tabWidget)
    , contextMenuTabIndex(-1)
    , plusTabIndex_(-1)
{
    // Подключаем сигналы
    connect(tabWidget, &QTabWidget::currentChanged, this, &TabManager::onTabChanged);
    connect(tabWidget->tabBar(), &QTabBar::customContextMenuRequested, this, &TabManager::showTabContextMenu);
}

TabManager::~TabManager()
{
}

void TabManager::initializeTabs()
{
    // Очищаем существующие вкладки кроме первой
    while (tabWidget->count() > 1) {
        tabWidget->removeTab(1);
    }
    tabInfos.clear();

    // Создаем начальную вкладку "Сканирование"
    TabInfo defaultTab;
    defaultTab.name = "Сканирование";
    defaultTab.deviceIndex = -1;
    tabInfos.push_back(defaultTab);

    // Настраиваем содержимое первой вкладки
    setupTabContent(0);

    // Загружаем сохранённые вкладки из конфига
    loadTabs();

    // Добавляем специальную вкладку с кнопкой "+"
    addPlusTab();
}

void TabManager::createNewTab(const QString& tabName)
{
    TabInfo newTabInfo;
    newTabInfo.name = tabName;
    newTabInfo.deviceIndex = -1;

    // Добавляем информацию о вкладке в вектор (перед последним элементом, если он есть)
    tabInfos.push_back(newTabInfo);

    // Создаем новую вкладку и вставляем перед вкладкой с плюсом
    QWidget* newTab = new QWidget();
    int insertIndex = plusTabIndex_; // Вставляем перед плюсом
    tabWidget->insertTab(insertIndex, newTab, tabName);

    // Обновляем индекс вкладки с плюсом (сдвигается вправо)
    plusTabIndex_++;

    // Настраиваем содержимое вкладки
    setupTabContent(insertIndex);

    // Переключаемся на новую вкладку
    tabWidget->setCurrentIndex(insertIndex);

    // Сохраняем изменения в конфиг
    saveTabs();

    emit tabCreated(tabName);

    qDebug() << "Created new tab at index:" << insertIndex << ", plus tab now at:" << plusTabIndex_;
}

void TabManager::deleteTab(int tabIndex)
{
    if (tabIndex >= 0 && tabIndex < tabWidget->count()) {
        // Не разрешаем удалять вкладку с плюсом
        if (tabIndex == plusTabIndex_) {
            QMessageBox::information(nullptr, "Удаление вкладки",
                                   "Нельзя удалить вкладку для создания новых вкладок");
            return;
        }

        // Не разрешаем удалять последнюю обычную вкладку
        if (static_cast<int>(tabInfos.size()) <= 1) {
            QMessageBox::information(nullptr, "Удаление вкладки",
                                   "Нельзя удалить последнюю вкладку");
            return;
        }

        // Подтверждаем удаление
        QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Удаление вкладки",
            "Вы действительно хотите удалить эту вкладку?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            tabWidget->removeTab(tabIndex);
            tabInfos.erase(tabInfos.begin() + tabIndex);

            // Если удаляемая вкладка была перед вкладкой с плюсом, обновляем индекс
            if (tabIndex < plusTabIndex_) {
                plusTabIndex_--;
            }

            // Сохраняем изменения в конфиг
            saveTabs();

            emit tabDeleted(tabIndex);

            qDebug() << "Deleted tab at index:" << tabIndex << ", plus tab now at:" << plusTabIndex_;
        }
    }
}

void TabManager::setupTabContent(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= tabWidget->count()) {
        return;
    }

    QWidget* tabWidgetPtr = tabWidget->widget(tabIndex);
    if (!tabWidgetPtr) return;

    // Очищаем содержимое вкладки
    clearTabContent(tabWidgetPtr);

    // Создаем горизонтальный layout для кнопок профилей
    QHBoxLayout* layout = new QHBoxLayout(tabWidgetPtr);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Добавляем растягивающийся элемент для центрирования
    layout->addStretch();

    // Добавляем существующие профили
    if (tabIndex < static_cast<int>(tabInfos.size())) {
        for (const auto& profile : tabInfos[tabIndex].profiles) {
            addProfileButton(tabIndex, profile);
        }
    }

    // Добавляем растягивающийся элемент для центрирования
    layout->addStretch();

    // Включаем контекстное меню для вкладки
    tabWidgetPtr->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabWidgetPtr, &QWidget::customContextMenuRequested, [this, tabIndex](const QPoint& pos) {
        showTabContextMenuForProfiles(tabIndex, pos);
    });
}

void TabManager::addProfileButton(int tabIndex, const ProfileManager::ScanProfile& profile)
{
    if (tabIndex < 0 || tabIndex >= tabWidget->count()) {
        return;
    }

    QWidget* tabWidgetPtr = tabWidget->widget(tabIndex);
    if (!tabWidgetPtr || !tabWidgetPtr->layout()) return;

    QPushButton* profileButton = new QPushButton(profile.buttonText);
    setupProfileButton(profileButton, profile);
    connectProfileButton(profileButton, profile);

    // Вставляем кнопку перед stretch элементом
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(tabWidgetPtr->layout());
    if (layout) {
        int insertIndex = layout->count() - 1; // Перед stretch
        layout->insertWidget(insertIndex, profileButton);
    }
}

void TabManager::loadTabs()
{
    Settings& settings = Settings::getInstance();
    QStringList savedTabNames = settings.getTabNames();

    qDebug() << "Loading saved tabs:" << savedTabNames;

    // Загружаем каждую сохранённую вкладку
    for (const QString& tabName : savedTabNames) {
        TabInfo tabInfo;
        tabInfo.name = tabName;
        tabInfo.deviceIndex = -1;

        // Загружаем профили для этой вкладки
        QVariantMap profilesMap = settings.getTabProfiles(tabName);
        for (auto it = profilesMap.begin(); it != profilesMap.end(); ++it) {
            QVariantMap profileData = it.value().toMap();
            
            ProfileManager::ScanProfile profile;
            profile.name = it.key();
            profile.colorMode = profileData.value("colorMode", "").toString();
            profile.resolution = profileData.value("resolution", "").toString();
            profile.scanArea = profileData.value("scanArea", "").toString();
            profile.outputPath = profileData.value("outputPath", "").toString();
            profile.filePrefix = profileData.value("filePrefix", "").toString();
            profile.fileFormat = profileData.value("fileFormat", "PREFIX_DATETIME").toString();
            profile.outputFormat = profileData.value("outputFormat", "PNG").toString();
            profile.quality = profileData.value("quality", 90).toInt();
            profile.buttonText = profileData.value("buttonText", profile.name).toString();
            
            tabInfo.profiles.push_back(profile);
        }

        // Добавляем вкладку в список
        tabInfos.push_back(tabInfo);

        // Создаем вкладку в UI
        QWidget* tabWidgetPtr = new QWidget();
        int tabIndex = tabWidget->addTab(tabWidgetPtr, tabName);
        
        // Настраиваем содержимое вкладки
        setupTabContent(tabIndex);

        qDebug() << "Loaded tab:" << tabName << "with" << tabInfo.profiles.size() << "profiles";
    }
}

void TabManager::saveTabs()
{
    Settings& settings = Settings::getInstance();
    
    // Собираем имена всех вкладок (кроме первой "Сканирование" и последней с плюсом)
    QStringList tabNames;
    for (int i = 1; i < static_cast<int>(tabInfos.size()); ++i) {
        tabNames.append(tabInfos[i].name);
    }

    // Сохраняем список имён вкладок
    settings.setTabNames(tabNames);

    // Сохраняем профили для каждой вкладки
    for (int i = 1; i < static_cast<int>(tabInfos.size()); ++i) {
        const TabInfo& tabInfo = tabInfos[i];
        
        QVariantMap profilesMap;
        for (const ProfileManager::ScanProfile& profile : tabInfo.profiles) {
            QVariantMap profileData;
            profileData["colorMode"] = profile.colorMode;
            profileData["resolution"] = profile.resolution;
            profileData["scanArea"] = profile.scanArea;
            profileData["outputPath"] = profile.outputPath;
            profileData["filePrefix"] = profile.filePrefix;
            profileData["fileFormat"] = profile.fileFormat;
            profileData["outputFormat"] = profile.outputFormat;
            profileData["quality"] = profile.quality;
            profileData["buttonText"] = profile.buttonText;
            
            profilesMap[profile.name] = profileData;
        }
        
        settings.setTabProfiles(tabInfo.name, profilesMap);
    }

    qDebug() << "Saved" << tabNames.size() << "tabs to config";
}

void TabManager::showTabContextMenu(const QPoint& pos)
{
    // Определяем индекс вкладки по позиции курсора
    int tabIndex = tabWidget->tabBar()->tabAt(pos);

    if (tabIndex >= 0) {
        contextMenuTabIndex = tabIndex;

        // Создаем контекстное меню
        QMenu* contextMenu = new QMenu();
        QAction* deleteAction = contextMenu->addAction("Удалить вкладку");

        connect(deleteAction, &QAction::triggered, [this]() {
            deleteTab(contextMenuTabIndex);
        });

        // Показываем меню
        contextMenu->exec(tabWidget->tabBar()->mapToGlobal(pos));
        delete contextMenu;
    }
}

void TabManager::showTabContextMenuForProfiles(int tabIndex, const QPoint& pos)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(tabInfos.size())) {
        return;
    }

    TabInfo& tabInfo = tabInfos[tabIndex];

    // Проверяем лимит кнопок (максимум 5)
    if (tabInfo.profiles.size() >= 5) {
        QMessageBox::information(nullptr, "Лимит профилей", 
                               "Максимальное количество профилей на вкладке: 5");
        return;
    }

    QMenu* contextMenu = new QMenu();
    QAction* createProfileAction = contextMenu->addAction("Создать профиль");

    connect(createProfileAction, &QAction::triggered, [this, tabIndex, pos]() {
        // Эмитируем сигнал для создания профиля
        emit profileContextMenuRequested(tabIndex, ProfileManager::ScanProfile(), pos);
    });

    // Показываем меню
    QWidget* tabWidgetPtr = tabWidget->widget(tabIndex);
    contextMenu->exec(tabWidgetPtr->mapToGlobal(pos));
    delete contextMenu;
}

void TabManager::createNewTabDialog()
{
    qDebug() << "createNewTabDialog called";
    
    QDialog dialog;
    dialog.setWindowTitle("Создать новую вкладку");
    dialog.setModal(true);
    dialog.resize(300, 150);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Поле для имени вкладки
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Имя вкладки:");
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setText("Новая вкладка");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    layout->addLayout(nameLayout);

    // Кнопки OK/Cancel
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Отмена");

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    qDebug() << "Showing dialog";
    
    if (dialog.exec() == QDialog::Accepted && !nameEdit->text().isEmpty()) {
        qDebug() << "Dialog accepted, creating tab:" << nameEdit->text();
        createNewTab(nameEdit->text());
    } else {
        qDebug() << "Dialog rejected or empty name";
    }
}

TabManager::TabInfo* TabManager::getTabInfo(int index)
{
    if (index >= 0 && index < static_cast<int>(tabInfos.size())) {
        return &tabInfos[index];
    }
    return nullptr;
}

int TabManager::getCurrentTabIndex() const
{
    return tabWidget->currentIndex();
}

void TabManager::addProfileToTab(int tabIndex, const ProfileManager::ScanProfile& profile)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int>(tabInfos.size())) {
        tabInfos[tabIndex].profiles.push_back(profile);
        addProfileButton(tabIndex, profile);
        saveTabs();
    }
}

void TabManager::removeProfileFromTab(int tabIndex, const QString& profileName)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int>(tabInfos.size())) {
        auto& profiles = tabInfos[tabIndex].profiles;
        profiles.erase(std::remove_if(profiles.begin(), profiles.end(),
            [&profileName](const ProfileManager::ScanProfile& p) { 
                return p.name == profileName; 
            }), profiles.end());
        
        // Перезагружаем вкладку
        setupTabContent(tabIndex);
        saveTabs();
    }
}

void TabManager::updateProfileInTab(int tabIndex, const ProfileManager::ScanProfile& profile)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int>(tabInfos.size())) {
        auto& profiles = tabInfos[tabIndex].profiles;
        for (auto& p : profiles) {
            if (p.name == profile.name) {
                p = profile;
                break;
            }
        }
        
        // Перезагружаем вкладку
        setupTabContent(tabIndex);
        saveTabs();
    }
}

void TabManager::addPlusTab()
{
    // Создаем пустой виджет для вкладки с плюсом
    QWidget* plusTabWidget = new QWidget();
    
    // Добавляем вкладку с заголовком "+"
    int plusTabIndex = tabWidget->addTab(plusTabWidget, "+");
    plusTabIndex_ = plusTabIndex;

    // Подключаем обработчик клика по заголовку вкладки
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == plusTabIndex_) {
            // Если выбрана вкладка с плюсом, открываем диалог и возвращаемся на предыдущую
            qDebug() << "Plus tab selected, opening dialog";
            createNewTabDialog();
            
            // Возвращаемся на предыдущую вкладку
            int prevIndex = (plusTabIndex_ > 0) ? plusTabIndex_ - 1 : 0;
            tabWidget->setCurrentIndex(prevIndex);
        }
    });

    qDebug() << "Added plus tab at index:" << plusTabIndex;
}

void TabManager::clearTabContent(QWidget* tabWidgetPtr)
{
    QLayout* existingLayout = tabWidgetPtr->layout();
    if (existingLayout) {
        QLayoutItem* item;
        while ((item = existingLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete existingLayout;
    }
}

void TabManager::setupProfileButton(QPushButton* button, const ProfileManager::ScanProfile& profile)
{
    button->setFixedSize(120, 120); // Большие квадратные кнопки
    button->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    padding: 8px;"
        "    text-align: center;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "    transform: scale(1.05);"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "    transform: scale(0.95);"
        "}"
    );

    // Включаем контекстное меню для кнопки профиля
    button->setContextMenuPolicy(Qt::CustomContextMenu);
}

void TabManager::connectProfileButton(QPushButton* button, const ProfileManager::ScanProfile& profile)
{
    connect(button, &QPushButton::clicked, [this, profile]() {
        emit profileButtonClicked(profile);
    });

    // Подключаем сигнал контекстного меню для редактирования профиля
    connect(button, &QPushButton::customContextMenuRequested, [this, profile](const QPoint& pos) {
        emit profileContextMenuRequested(-1, profile, pos);
    });
}

void TabManager::onTabChanged(int index)
{
    // Игнорируем вкладку с плюсом - она обрабатывается в addPlusTab()
    if (index < 0 || index == plusTabIndex_) {
        return;
    }

    // Проверяем, что индекс находится в допустимых пределах
    if (index >= static_cast<int>(tabInfos.size())) {
        qDebug() << "Tab index out of range:" << index << ", tabInfos size:" << tabInfos.size();
        return;
    }

    emit tabChanged(index);
}

void TabManager::onPlusTabClicked()
{
    createNewTabDialog();
}

void TabManager::onProfileButtonClicked()
{
    // Обработка будет в MainWindow
}

void TabManager::onProfileContextMenuRequested()
{
    // Обработка будет в MainWindow
}
