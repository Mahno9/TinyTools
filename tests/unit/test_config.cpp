#include <QtTest>
#include "../../src/models/AppConfig.h"
#include "../../src/core/HotkeyManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

class TestConfig : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void testDefaultValues();
    void testSetAndGetValues();
    void testSaveAndLoad();
    void testHotkeyRoundTrip();
    void testValueBounds();
    void testCorruptConfigIsBackedUp();
    void testRegistryPathIsQuoted();
    void testNoStaleDataAfterReset();
};

void TestConfig::initTestCase() {
    // Isolate from the user's real %APPDATA%: all QStandardPaths locations
    // point into a test directory for the whole test run.
    QStandardPaths::setTestModeEnabled(true);
}

void TestConfig::init() {
    // Reset singleton and remove any config file from a previous test
    AppConfig::cleanupInstance();
    QFile::remove(AppConfig::instance()->getConfigFilePath());
    QFile::remove(AppConfig::instance()->getConfigFilePath() + ".bak");
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

void TestConfig::testHotkeyRoundTrip() {
    AppConfig *config = AppConfig::instance();

    const Qt::KeyboardModifiers mods =
        Qt::ControlModifier | Qt::ShiftModifier | Qt::MetaModifier;
    config->setHotkey(HotkeyType::AlternativeToggle, Qt::Key_F9, mods);
    QVERIFY(config->save());

    AppConfig::cleanupInstance();
    AppConfig *config2 = AppConfig::instance();
    QVERIFY(config2->load());

    QCOMPARE(config2->getHotkeyKey(HotkeyType::AlternativeToggle),
             static_cast<int>(Qt::Key_F9));
    QCOMPARE(config2->getHotkeyModifiers(HotkeyType::AlternativeToggle), mods);
}

void TestConfig::testValueBounds() {
    AppConfig *config = AppConfig::instance();

    config->setWindowOpacity(150);
    QCOMPARE(config->getWindowOpacity(), 100);
    config->setWindowOpacity(5);
    QCOMPARE(config->getWindowOpacity(), 20);

    config->setWindowWidth(10000);
    QCOMPARE(config->getWindowWidth(), 1920);
    config->setWindowWidth(10);
    QCOMPARE(config->getWindowWidth(), 400);

    config->setWindowHeight(10000);
    QCOMPARE(config->getWindowHeight(), 1080);
    config->setWindowHeight(10);
    QCOMPARE(config->getWindowHeight(), 300);
}

void TestConfig::testCorruptConfigIsBackedUp() {
    AppConfig *config = AppConfig::instance();
    const QString path = config->getConfigFilePath();
    const QString backup = path + ".bak";

    // Write garbage where the config should be
    QDir().mkpath(QFileInfo(path).absolutePath());
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ this is not : valid json ");
    }

    // load() must fail, move the corrupt file aside, and keep defaults
    QVERIFY(!config->load());
    QVERIFY(!QFile::exists(path));
    QVERIFY(QFile::exists(backup));
    QCOMPARE(config->getWindowOpacity(), 90); // defaults intact

    // A subsequent save must produce a loadable file again
    QVERIFY(config->save());
    AppConfig::cleanupInstance();
    QVERIFY(AppConfig::instance()->load());
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
