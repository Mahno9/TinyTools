#pragma once

#include <QString>
#include <QRect>
#include <QScreen>
#include <QApplication>

namespace Utils {

    /**
     * @brief Get the application data directory path
     * @return Path to the application data directory
     */
    QString getAppDataDir();

    /**
     * @brief Get the configuration file path
     * @return Path to the configuration file
     */
    QString getConfigFilePath();

    /**
     * @brief Ensure a directory exists, create if it doesn't
     * @param path Directory path
     * @return true if directory exists or was created successfully
     */
    bool ensureDirExists(const QString& path);

    /**
     * @brief Trim whitespace from both ends of a string
     * @param str String to trim
     * @return Trimmed string
     */
    QString trimWhitespace(const QString& str);

    /**
     * @brief Escape HTML special characters in a string
     * @param str String to escape
     * @return HTML-escaped string
     */
    QString escapeHtml(const QString& str);

    /**
     * @brief Truncate a string to a maximum length with ellipsis
     * @param str String to truncate
     * @param maxLength Maximum length
     * @return Truncated string
     */
    QString truncateString(const QString& str, int maxLength);

    /**
     * @brief Check if a string is null or empty
     * @param str String to check
     * @return true if string is null or empty
     */
    bool isNullOrEmpty(const QString& str);

    /**
     * @brief Convert Qt::KeyboardModifiers to a string representation
     * @param modifiers Keyboard modifiers
     * @return String representation (e.g., "Ctrl+Alt")
     */
    QString keyboardModifiersToString(Qt::KeyboardModifiers modifiers);

    /**
     * @brief Convert a key code to a readable string
     * @param key Qt key code
     * @return String representation (e.g., "T", "F1")
     */
    QString keyCodeToString(int key);

    /**
     * @brief Center a rectangle on a screen
     * @param rect Rectangle to center
     * @param screen Screen to center on (default: primary screen)
     * @return Centered rectangle
     */
    QRect centerRectOnScreen(const QRect& rect, QScreen* screen = nullptr);

    /**
     * @brief Ensure a rectangle is visible on screen
     * @param rect Rectangle to check
     * @param screen Screen to check against (default: primary screen)
     * @return Adjusted rectangle that is visible on screen
     */
    QRect ensureRectVisible(const QRect& rect, QScreen* screen = nullptr);

    /**
     * @brief Get the primary screen geometry
     * @return Primary screen geometry
     */
    QRect getPrimaryScreenGeometry();

    /**
     * @brief Check if a point is within the screen bounds
     * @param point Point to check
     * @return true if point is visible on any screen
     */
    bool isPointVisible(const QPoint& point);

    /**
     * @brief Format a file size to human-readable string
     * @param bytes File size in bytes
     * @return Formatted string (e.g., "1.5 MB")
     */
    QString formatFileSize(qint64 bytes);

    /**
     * @brief Get a safe filename from a potentially unsafe string
     * @param filename Original filename
     * @return Safe filename with invalid characters replaced
     */
    QString sanitizeFilename(const QString& filename);

    /**
     * @brief Join strings with a separator
     * @param strings List of strings to join
     * @param separator Separator string
     * @return Joined string
     */
    QString joinStrings(const QStringList& strings, const QString& separator);

    /**
     * @brief Split a string into lines
     * @param str String to split
     * @return List of lines
     */
    QStringList splitLines(const QString& str);

    /**
     * @brief Remove duplicate lines from a string list
     * @param lines List of lines
     * @return List with duplicates removed
     */
    QStringList removeDuplicateLines(const QStringList& lines);

    /**
     * @brief Get the current timestamp as a formatted string
     * @param format Format string (default: "yyyy-MM-dd hh:mm:ss")
     * @return Formatted timestamp
     */
    QString getTimestamp(const QString& format = "yyyy-MM-dd hh:mm:ss");

    /**
     * @brief Get the application version as a formatted string
     * @param major Major version
     * @param minor Minor version
     * @param patch Patch version
     * @return Version string (e.g., "1.0.0")
     */
    QString formatVersion(int major, int minor, int patch);

    /**
     * @brief Parse a version string into major, minor, patch components
     * @param version Version string (e.g., "1.0.0")
     * @return Tuple of (major, minor, patch)
     */
    QTuple<int, int, int> parseVersion(const QString& version);

} // namespace Utils
