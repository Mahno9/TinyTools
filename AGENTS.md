# AGENTS.md

This file provides guidance for agentic coding assistants working on the TinyTools C++/Qt project.

## Build Commands

### Build the project
```bash
cd build
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.3.0\msvc2019_64" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### Build debug version
```bash
cmake --build . --config Debug
```

### Clean build
```bash
cmake --build . --target clean
rm -rf CMakeCache.txt CMakeFiles
```

### Deploy Qt dependencies
```bash
cd build/Release
windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw TinyTools.exe
```

## Testing

### Run all tests
```bash
cd build
ctest --config Release --output-on-failure
```

### Run a single test file
```bash
cd build/tests/unit
./test_config.exe
./test_clipboard.exe
./test_hotkey.exe
```

### Run integration tests
```bash
cd build/tests/integration
./test_webview.exe
```

## Code Style Guidelines

### Naming Conventions
- **Classes**: PascalCase (e.g., `ClipboardManager`, `MainWindow`)
- **Functions/Methods**: camelCase (e.g., `getText()`, `initialize()`)
- **Member Variables**: `m_` prefix with camelCase (e.g., `m_lastText`, `m_webView`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_TEXT_LENGTH`, `DEFAULT_WIDTH`)
- **Signals**: camelCase (e.g., `clipboardChanged()`, `hotkeyPressed()`)

### File Organization
Use `#pragma once` in headers. Structure: Q_OBJECT, public, public slots, signals, protected, private slots, private methods, member variables, Q_DISABLE_COPY.

```cpp
#pragma once
#include <QObject>

class MyClass : public QObject {
    Q_OBJECT
public:
    explicit MyClass(QObject* parent = nullptr);
private slots:
    void handleEvent();
signals:
    void valueChanged(const QString& value);
private:
    QString m_value;
    Q_DISABLE_COPY(MyClass)
};
```

### Memory Management
- Use `QPointer<T>` for QObject-owned pointers
- Use Qt's parent-child system for automatic cleanup
- Use `std::unique_ptr<T>` for non-QObject types
- Avoid manual `new`/`delete` when possible

### Signal-Slot Connections
Use modern Qt5+ syntax with function pointers. Use `Qt::UniqueConnection` to avoid duplicates.
```cpp
connect(sender, &Sender::signalName, receiver, &Receiver::slotName);
connect(sender, &Sender::signal, receiver, &Receiver::slot, Qt::UniqueConnection);
```

### Error Handling
- Use `try-catch` for exception-prone code
- Use Qt logging: `qDebug()`, `qInfo()`, `qWarning()`, `qCritical()`
- Return `bool` for success/failure
- Check return values and handle failures gracefully

### Constants and Configuration
Define constants in `src/app/Constants.h` using `constexpr`. Use `QSettings` for config persistence. Store config in `%APPDATA%\TinyTools\settings.json`.

### Formatting
- 4-space indentation (no tabs)
- Opening brace on new line for functions/classes
- Opening brace on same line for control structures
- Spaces around operators
- Maximum line length: 120 characters

### Platform-Specific Code
Use `#ifdef _WIN32` for Windows, `#ifdef __linux__` for Linux. Keep platform implementations minimal.

### Qt Best Practices
- Set `Q_OBJECT` macro first in QObject-derived classes
- Use `explicit` for single-argument constructors
- Override virtual functions with `override` keyword
- Use `Q_DISABLE_COPY` macro for QObject classes
- Prefer Qt containers (QString, QList, QVector) over STL

### Testing Guidelines
- Use QtTest framework with `test_*.cpp` naming in `tests/unit/` or `tests/integration/`
- Use `QVERIFY()`, `QCOMPARE()` for assertions
- Use `QSignalSpy` for signal testing
- Clean up resources in `cleanupTestCase()`

## Architecture Notes
- **MVC Pattern**: UI layer, business logic (core/), data models (models/)
- **Signal-Slot**: Observer pattern for loose coupling
- **Singleton**: Optional pattern for configuration manager
- **Factory**: Component creation in Application class
- Main entry: `src/main.cpp` creates `Application` instance
- Config stored via `AppConfig` class in JSON
- WebView uses Qt WebEngine (Chromium-based)

## Development Workflow
1. Make changes following code style guidelines
2. Build: `cmake --build . --config Release`
3. Run tests: `ctest --config Release --output-on-failure`
4. Test manually if needed
5. Deploy: `windeployqt TinyTools.exe`

## No Lint/Typecheck Configured
Rely on compiler warnings, Qt Creator's code analysis, and manual code review.
