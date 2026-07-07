cmake_minimum_required(VERSION 3.22)
project(TinyTools VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WebEngineCore
    WebEngineWidgets
)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/app/Logging.cpp
    src/core/ClipboardManager.cpp
    src/core/HotkeyManager.cpp
    src/ui/MainWindow.cpp
    src/ui/SettingsDialog.cpp
    src/ui/WebViewContainer.cpp
    src/ui/HotkeyEdit.cpp
    src/tray/TrayIcon.cpp
    src/models/AppConfig.cpp
    src/models/ResourceManager.cpp
    src/resources/icons/resources.qrc
)

set(HEADERS
    src/app/Application.h
    src/app/Constants.h
    src/app/Logging.h
    src/core/ClipboardManager.h
    src/core/HotkeyManager.h
    src/ui/MainWindow.h
    src/ui/SettingsDialog.h
    src/ui/WebViewContainer.h
    src/ui/HotkeyEdit.h
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

# ── Unit Tests ────────────────────────────────────────────────────────────────
enable_testing()
find_package(Qt6 COMPONENTS Test REQUIRED)

# Shared source files compiled into every test (no main, no WebEngine UI deps)
set(TEST_SHARED_SOURCES
    src/app/Logging.cpp
    src/models/AppConfig.cpp
    src/models/ResourceManager.cpp
    src/core/HotkeyManager.cpp
    src/core/ClipboardManager.cpp
    src/ui/HotkeyEdit.cpp
)

function(add_unit_test TEST_NAME)
    add_executable(${TEST_NAME}
        tests/unit/${TEST_NAME}.cpp
        ${TEST_SHARED_SOURCES}
    )
    target_include_directories(${TEST_NAME} PRIVATE src)
    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
    )
    if(WIN32)
        target_link_libraries(${TEST_NAME} PRIVATE dwmapi)
    endif()
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    # Offscreen platform: tests must not depend on a visible desktop session.
    set_tests_properties(${TEST_NAME} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    if(WIN32)
        # Test executables need Qt DLLs on PATH (no windeployqt for tests)
        get_target_property(QT_CORE_DLL Qt6::Core LOCATION)
        get_filename_component(QT_BIN_DIR "${QT_CORE_DLL}" DIRECTORY)
        set_tests_properties(${TEST_NAME} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${QT_BIN_DIR}")
    endif()
endfunction()

add_unit_test(test_webresource)
add_unit_test(test_config)
add_unit_test(test_resource_manager)
add_unit_test(test_logging)
add_unit_test(test_hotkey)
add_unit_test(test_hotkeyedit)
add_unit_test(test_clipboard)
