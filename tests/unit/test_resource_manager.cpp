#include <QtTest>
#include <QSignalSpy>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>
#include "../../src/models/ResourceManager.h"
#include "../../src/models/AppConfig.h"
#include "../../src/app/Constants.h"

class TestResourceManager : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testAddResource();
    void testAddResourceEmitsSignals();
    void testRemoveResource();
    void testUpdateResource();
    void testGetByIdReturnsInvalidForUnknown();
    void testClearAllResources();
    void testImportCapAtMaxResources();
    void testImportRejectsNonHttpUrls();
    void testCleanupInstanceResetsState();
};

static WebResource makeResource(const QString &name, const QString &url) {
    return WebResource::create(name, url);
}

static bool writePresetFile(QTemporaryFile &file, const QJsonArray &resources) {
    QJsonObject root;
    root["version"]   = "1.0";
    root["resources"] = resources;
    QJsonDocument doc(root);
    file.open();
    file.write(doc.toJson());
    file.flush();
    file.close();
    return true;
}

void TestResourceManager::init() {
    AppConfig::cleanupInstance();
    ResourceManager::cleanupInstance();
    AppConfig::instance()->resetToDefaults();
}

void TestResourceManager::cleanup() {
    ResourceManager::cleanupInstance();
    AppConfig::cleanupInstance();
}

void TestResourceManager::testAddResource() {
    auto *rm = ResourceManager::instance();
    QCOMPARE(rm->getResourceCount(), 0);

    WebResource r = makeResource("Test", "https://example.com");
    rm->addResource(r);
    QCOMPARE(rm->getResourceCount(), 1);
    QCOMPARE(rm->getResourceById(r.id).name, QString("Test"));
}

void TestResourceManager::testAddResourceEmitsSignals() {
    auto *rm = ResourceManager::instance();
    QSignalSpy spyAdded(rm, &ResourceManager::resourceAdded);
    QSignalSpy spyChanged(rm, &ResourceManager::resourcesChanged);

    WebResource r = makeResource("Sig", "https://sig.example.com");
    rm->addResource(r);

    QCOMPARE(spyAdded.count(), 1);
    QCOMPARE(spyChanged.count(), 1);
}

void TestResourceManager::testRemoveResource() {
    auto *rm = ResourceManager::instance();
    WebResource r = makeResource("Del", "https://del.example.com");
    rm->addResource(r);
    QCOMPARE(rm->getResourceCount(), 1);

    rm->removeResource(r.id);
    QCOMPARE(rm->getResourceCount(), 0);
    QVERIFY(!rm->getResourceById(r.id).isValid());
}

void TestResourceManager::testUpdateResource() {
    auto *rm = ResourceManager::instance();
    WebResource r = makeResource("Old", "https://old.example.com");
    rm->addResource(r);

    r.name = "New";
    rm->updateResource(r);
    QCOMPARE(rm->getResourceById(r.id).name, QString("New"));
}

void TestResourceManager::testGetByIdReturnsInvalidForUnknown() {
    auto *rm = ResourceManager::instance();
    WebResource r = rm->getResourceById("does-not-exist");
    QVERIFY(!r.isValid());
}

void TestResourceManager::testClearAllResources() {
    auto *rm = ResourceManager::instance();
    rm->addResource(makeResource("A", "https://a.com"));
    rm->addResource(makeResource("B", "https://b.com"));
    QCOMPARE(rm->getResourceCount(), 2);

    rm->clearAllResources();
    QCOMPARE(rm->getResourceCount(), 0);
}

void TestResourceManager::testImportCapAtMaxResources() {
    auto *rm = ResourceManager::instance();

    // Build a preset JSON with MAX_RESOURCES + 5 entries
    const int total = Constants::MAX_RESOURCES + 5;
    QJsonArray arr;
    for (int i = 0; i < total; ++i) {
        QJsonObject obj;
        obj["name"]       = QString("Resource %1").arg(i);
        obj["url"]        = QString("https://example%1.com").arg(i);
        obj["isEnabled"]  = true;
        obj["order"]      = i;
        obj["zoomFactor"] = 1.0;
        arr.append(obj);
    }

    QTemporaryFile tmp;
    tmp.setAutoRemove(true);
    writePresetFile(tmp, arr);

    bool ok = rm->importPresets(tmp.fileName());
    QVERIFY(ok);
    QVERIFY(rm->getResourceCount() <= Constants::MAX_RESOURCES);
}

void TestResourceManager::testImportRejectsNonHttpUrls() {
    auto *rm = ResourceManager::instance();

    QJsonArray arr;
    // Valid entry
    QJsonObject valid;
    valid["name"]       = "Valid";
    valid["url"]        = "https://valid.com";
    valid["isEnabled"]  = true;
    valid["order"]      = 0;
    valid["zoomFactor"] = 1.0;
    arr.append(valid);

    // file:// entry — must be rejected by isValid()
    QJsonObject bad;
    bad["name"]       = "Bad";
    bad["url"]        = "file:///C:/Windows";
    bad["isEnabled"]  = true;
    bad["order"]      = 1;
    bad["zoomFactor"] = 1.0;
    arr.append(bad);

    QTemporaryFile tmp;
    tmp.setAutoRemove(true);
    writePresetFile(tmp, arr);

    rm->importPresets(tmp.fileName());

    // Only the valid https:// entry should have been imported
    QCOMPARE(rm->getResourceCount(), 1);
    QCOMPARE(rm->getAllResources().first().name, QString("Valid"));
}

void TestResourceManager::testCleanupInstanceResetsState() {
    auto *rm = ResourceManager::instance();
    rm->addResource(makeResource("Persist", "https://persist.com"));
    QCOMPARE(rm->getResourceCount(), 1);

    ResourceManager::cleanupInstance();

    // New instance must start empty
    auto *rm2 = ResourceManager::instance();
    QCOMPARE(rm2->getResourceCount(), 0);
}

QTEST_GUILESS_MAIN(TestResourceManager)
#include "test_resource_manager.moc"
