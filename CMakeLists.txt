cmake_minimum_required(VERSION 3.5)

project(AdvancedScan VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# set(TS_FILES AdvancedScan_ru_RU.ts)

set(PROJECT_SOURCES
        main.cpp
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/ui/mainwindow.ui
        src/ui/settingswindow.h
        src/ui/settingswindow.cpp
        src/core/cscanfront.h 
        src/core/cscanfront.cpp
        src/core/cscanner.h 
        src/core/cscanner.cpp
        src/core/settings.h 
        src/core/settings.cpp
        src/managers/scannermanager.h
        src/managers/scannermanager.cpp
        src/managers/profilemanager.h
        src/managers/profilemanager.cpp
        src/managers/tabmanager.h
        src/managers/tabmanager.cpp
        src/managers/filemanager.h
        src/managers/filemanager.cpp
        resources.qrc
        # ${TS_FILES}
)

# Устанавливаем выходную директорию для исполняемого файла
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AdvancedScan
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AdvancedScan APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    # qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(AdvancedScan SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AdvancedScan
            ${PROJECT_SOURCES}
        )
    endif()

    # qt5_add_translation(QM_FILES ${TS_FILES})
endif()

target_link_libraries(AdvancedScan PRIVATE Qt${QT_VERSION_MAJOR}::Widgets pthread)

set_target_properties(AdvancedScan PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AdvancedScan)
endif()
