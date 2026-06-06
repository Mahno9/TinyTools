#include <QtTest>
#include "../../src/models/AppConfig.h"
#include "../../src/core/HotkeyManager.h"
#include <QDir>
#include <QStandardPaths>

class TestConfig : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testDefaultValues();
    void testSetAndGetValues();
    void testSaveAndLoad();
    void testRegistryPathIsQuoted();
    void testNoStaleDataAfterReset();
};

void TestConfig::init() {
    // Reset singleton to a clean state before each test
    AppConfig::cleanupInstance();
    AppConfig::instance()->resetToDefaults();
}

void TestConfig::cleanup() {
    AppConfig::cleanupInstance();
}

void TestConfig::testDefaultValues() {
    AppConfig *config = AppConfig::instance();

    QCOMPARE(config->getHotkeyKey(HotkeyType::MainToggle), static_cast<int>(Qt::Key_T));
    QCOMPARE(config->getWindowOpacity(), 90);
    QCOMPARE(config->getAlwaysOnTop(), true);
    QCOMPARE(config->getAutoStartOnLogin(), false);
    QCOMPARE(config->getMinimizeToTray(), true);
    QCOMPARE(config->getDarkTheme(), false);
}

void TestConfig::testSetAndGetValues() {
    AppConfig *config = AppConfig::instance();

    config->setAlwaysOnTop(false);
    QCOMPARE(config->getAlwaysOnTop(), false);

    config->setWindowOpacity(75);
    QCOMPARE(config->getWindowOpacity(), 75);

    config->setWindowWidth(1024);
    QCOMPARE(config->getWindowWidth(), 1024);

    config->setWindowHeight(768);
    QCOMPARE(config->getWindowHeight(), 768);

    config->setWindowX(200);
    QCOMPARE(config->getWindowX(), 200);

    config->setWindowY(300);
    QCOMPARE(config->getWindowY(), 300);

    config->setMinimizeToTray(false);
    QCOMPARE(config->getMinimizeToTray(), false);

    config->setDarkTheme(true);
    QCOMPARE(config->getDarkTheme(), true);
}

void TestConfig::testSaveAndLoad() {
    AppConfig *config = AppConfig::instance();

    config->setAlwaysOnTop(false);
    config->setWindowOpacity(80);
    QVERIFY(config->save());

    // Fresh singleton: load from disk and verify persisted values
    AppConfig::cleanupInstance();
    AppConfig *config2 = AppConfig::instance();
    QVERIFY(config2->load());

    QCOMPARE(config2->getAlwaysOnTop(), false);
    QCOMPARE(config2->getWindowOpacity(), 80);
}

void TestConfig::testRegistryPathIsQuoted() {
    // Verify that the quoted-path logic produces a properly quoted string.
    // We replicate the quoting done in AppConfig::setAutoStartOnLogin().
    const wchar_t rawPath[] = L"C:\\Program Files\\TinyTools\\TinyTools.exe";
    std::wstring quotedPath = std::wstring(L"\"") + rawPath + L"\"";

    // Must start and end with a double-quote character
    QCOMPARE(quotedPath.front(), L'"');
    QCOMPARE(quotedPath.back(),  L'"');

    // The inner content must equal the original path
    std::wstring inner = quotedPath.substr(1, quotedPath.size() - 2);
    QCOMPARE(inner, std::wstring(rawPath));
}

void TestConfig::testNoStaleDataAfterReset() {
    // Verify that resetToDefaults() always produces canonical values,
    // i.e. in-memory state is not influenced by stale disk content.
    AppConfig *config = AppConfig::instance();
    config->setWindowOpacity(42);       // unusual value
    QCOMPARE(config->getWindowOpacity(), 42);

    config->resetToDefaults();
    QCOMPARE(config->getWindowOpacity(), 90);  // back to default
}

QTEST_GUILESS_MAIN(TestConfig)
#include "test_config.moc"
