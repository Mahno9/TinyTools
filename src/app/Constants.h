#pragma once

#include <QString>

namespace Constants {

// Application Info
constexpr const char *APP_NAME = "TinyTools";
constexpr const char *APP_VERSION = "1.0.0";
constexpr const char *ORGANIZATION_NAME = "TinyTools";

// Resource Settings
constexpr int MAX_RESOURCES = 10;

// Default Window Settings
constexpr int DEFAULT_WINDOW_WIDTH = 800;
constexpr int DEFAULT_WINDOW_HEIGHT = 600;
constexpr int DEFAULT_WINDOW_X = 100;
constexpr int DEFAULT_WINDOW_Y = 100;
constexpr int DEFAULT_OPACITY = 90;

// Window Constraints
constexpr int MIN_WINDOW_WIDTH = 400;
constexpr int MAX_WINDOW_WIDTH = 1920;
constexpr int MIN_WINDOW_HEIGHT = 300;
constexpr int MAX_WINDOW_HEIGHT = 1080;
constexpr int MIN_OPACITY = 20;
constexpr int MAX_OPACITY = 100;

// Default Hotkey
constexpr int DEFAULT_HOTKEY_KEY = 0x54; // T key
constexpr int DEFAULT_HOTKEY_MODIFIER_CTRL = 0x0002;
constexpr int DEFAULT_HOTKEY_MODIFIER_ALT = 0x0001;

// Clipboard Settings
constexpr int MAX_CLIPBOARD_TEXT_LENGTH = 10000000; // 10MB limit
constexpr int DEFAULT_CLIPBOARD_TRIM_LENGTH = 10000;

// WebView Settings
constexpr int WEBVIEW_LOAD_TIMEOUT_MS = 30000;
constexpr int WEBVIEW_RETRY_DELAY_MS = 1000;

// Network Settings
constexpr int NETWORK_CHECK_INTERVAL_MS = 5000;

// Icon Paths
constexpr const char *ICON_TRAY = ":/icons/tray.ico";
constexpr const char *ICON_TRAY_ACTIVE = ":/icons/tray-active.ico";
constexpr const char *ICON_APP = ":/icons/app.ico";

// Configuration
constexpr const char *CONFIG_FILE_NAME = "settings.json";

// Error Messages
constexpr const char *ERROR_CLIPBOARD_EMPTY =
    "Clipboard is empty or contains no valid text";
constexpr const char *ERROR_NETWORK_OFFLINE = "Network connection is offline";
constexpr const char *ERROR_WEBVIEW_LOAD_FAILED =
    "Failed to load resource page";
constexpr const char *ERROR_CONFIG_LOAD_FAILED =
    "Failed to load configuration file";
constexpr const char *ERROR_HOTKEY_REGISTER_FAILED =
    "Failed to register hotkey";

// Log Categories
constexpr const char *LOG_CATEGORY_APP = "app";
constexpr const char *LOG_CATEGORY_CLIPBOARD = "clipboard";
constexpr const char *LOG_CATEGORY_HOTKEY = "hotkey";
constexpr const char *LOG_CATEGORY_NETWORK = "network";
constexpr const char *LOG_CATEGORY_WEBVIEW = "webview";
constexpr const char *LOG_CATEGORY_CONFIG = "config";

} // namespace Constants
