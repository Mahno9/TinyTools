#include <QtTest>
#include "../../src/models/AppConfig.h"
#include <QFile>
#include <QDir>

class TestConfig : public QObject {
    Q_OBJECT
    
private slots:
    void testDefaultValues();
    void testSetAndGetValues();
    void testSaveAndLoad();
    void cleanupTestCase();
};

void TestConfig::testDefaultValues() {
    AppConfig config;
    
    QCOMPARE(config.getHotkeyKey(), static_cast<int>(Qt::Key_T));
    QCOMPARE(config.getWindowOpacity(), 90);
    QCOMPARE(config.getAlwaysOnTop(), true);
    QCOMPARE(config.getAutoStart(), true);
    QCOMPARE(config.getMinimizeToTray(), true);
}

void TestConfig::testSetAndGetValues() {
    AppConfig config;
    
    // Test window settings
    config.setAlwaysOnTop(false);
    QCOMPARE(config.getAlwaysOnTop(), false);
    
    config.setWindowOpacity(75);
    QCOMPARE(config.getWindowOpacity(), 75);
    
    config.setWindowWidth(1024);
    QCOMPARE(config.getWindowWidth(), 1024);
    
    config.setWindowHeight(768);
    QCOMPARE(config.getWindowHeight(), 768);
    
    config.setWindowX(200);
    QCOMPARE(config.getWindowX(), 200);
    
    config.setWindowY(300);
    QCOMPARE(config.getWindowY(), 300);
    
    // Test general settings
    config.setAutoStart(false);
    QCOMPARE(config.getAutoStart(), false);
    
    config.setMinimizeToTray(false);
    QCOMPARE(config.getMinimizeToTray(), false);
}

void TestConfig::testSaveAndLoad() {
    AppConfig config1;
    
    // Set some values
    config1.setAlwaysOnTop(false);
    config1.setWindowOpacity(80);
    
    // Save to temporary location
    QVERIFY(config1.save());
    
    // Load into another config instance
    AppConfig config2;
    QVERIFY(config2.load());
    
    // Verify values match
    QCOMPARE(config2.getAlwaysOnTop(), config1.getAlwaysOnTop());
    QCOMPARE(config2.getWindowOpacity(), config1.getWindowOpacity());
}

void TestConfig::cleanupTestCase() {
    // Clean up test config files
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(configDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }
}

QTEST_MAIN(TestConfig)
#include "test_config.moc"
