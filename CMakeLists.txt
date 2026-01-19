cmake_minimum_required(VERSION 3.16)
project(TinyTools VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WebEngineCore
    WebEngineWidgets
    Network
)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/core/ClipboardManager.cpp
    src/core/HotkeyManager.cpp
    src/core/NetworkMonitor.cpp
    src/ui/MainWindow.cpp
    src/ui/SettingsDialog.cpp
    src/ui/WebViewContainer.cpp
    src/tray/TrayIcon.cpp
    src/models/AppConfig.cpp
    src/models/ResourceManager.cpp
    src/resources/icons/resources.qrc
)

set(HEADERS
    src/app/Application.h
    src/app/Constants.h
    src/core/ClipboardManager.h
    src/core/HotkeyManager.h
    src/core/NetworkMonitor.h
    src/ui/MainWindow.h
    src/ui/SettingsDialog.h
    src/ui/WebViewContainer.h
    src/tray/TrayIcon.h
    src/models/AppConfig.h
    src/models/WebResource.h
    src/models/ResourceManager.h
)

add_executable(TinyTools WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(TinyTools PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::Network
)

if(WIN32)
    target_link_libraries(TinyTools PRIVATE dwmapi)
endif()

# Windows-specific settings
if(WIN32)
    set_target_properties(TinyTools PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "TinyTools"
    )
    
    # Set application icon
    set(APP_ICON_RESOURCE "${CMAKE_SOURCE_DIR}/src/resources/icons/app.rc")
    if(EXISTS ${APP_ICON_RESOURCE})
        target_sources(TinyTools PRIVATE ${APP_ICON_RESOURCE})
    endif()
endif()

