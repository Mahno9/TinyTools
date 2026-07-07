#include <QtTest>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryFile>
#include "../../src/models/ResourceManager.h"
#include "../../src/models/AppConfig.h"
#include "../../src/app/Constants.h"

class TestResourceManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void testAddResource();
    void testAddResourceEmitsSignals();
    void testRemoveResource();
    void testUpdateResource();
    void testGetByIdReturnsInvalidForUnknown();
    void testClearAllResources();
    void testSetResourceZoomIsSilent();
    void testSaveLoadRoundTrip();
    void testReorderResources();
    void testStartupResourceFallback();
    void testImportCapAtMaxResources();
    void testImportRejectsNonHttpUrls();
    void testImportRegeneratesDuplicateIds();
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

void TestResourceManager::initTestCase() {
    // Isolate from the user's real %APPDATA%
    QStandardPaths::setTestModeEnabled(true);
}

void TestResourceManager::init() {
    AppConfig::cleanupInstance();
    ResourceManager::cleanupInstance();
    QFile::remove(AppConfig::instance()->getConfigFilePath());
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

void TestResourceManager::testSetResourceZoomIsSilent() {
    auto *rm = ResourceManager::instance();
    WebResource r = makeResource("Zoom", "https://zoom.example.com");
    rm->addResource(r);

    QSignalSpy spyChanged(rm, &ResourceManager::resourcesChanged);
    rm->setResourceZoom(r.id, 1.7);

    QCOMPARE(spyChanged.count(), 0); // no tab-rebuild churn per wheel tick
    QCOMPARE(rm->getResourceById(r.id).zoomFactor, 1.7);
}

void TestResourceManager::testSaveLoadRoundTrip() {
    auto *rm = ResourceManager::instance();

    WebResource a = makeResource("Alpha", "https://a.example.com");
    a.openScript = "console.log(window.tinyToolsClipboard);";
    a.zoomFactor = 1.5;
    WebResource b = makeResource("Beta", "https://b.example.com");
    b.isEnabled = false;
    rm->addResource(a);
    rm->addResource(b);

    rm->setStartupMode(ResourceManager::SelectedResource);
    rm->setDefaultResourceId(a.id);
    rm->setLastUsedResourceId(b.id);

    QVERIFY(rm->saveToConfig());

    // Fresh manager, same AppConfig in memory: must restore everything
    ResourceManager::cleanupInstance();
    auto *rm2 = ResourceManager::instance();
    QVERIFY(rm2->loadFromConfig());

    QCOMPARE(rm2->getResourceCount(), 2);
    QCOMPARE(rm2->getResourceById(a.id).openScript, a.openScript);
    QCOMPARE(rm2->getResourceById(a.id).zoomFactor, 1.5);
    QCOMPARE(rm2->getResourceById(b.id).isEnabled, false);
    QCOMPARE(rm2->getStartupMode(), ResourceManager::SelectedResource);
    QCOMPARE(rm2->getDefaultResourceId(), a.id);
    QCOMPARE(rm2->getLastUsedResourceId(), b.id);
}

void TestResourceManager::testReorderResources() {
    auto *rm = ResourceManager::instance();
    WebResource a = makeResource("A", "https://a.com");
    WebResource b = makeResource("B", "https://b.com");
    WebResource c = makeResource("C", "https://c.com");
    rm->addResource(a);
    rm->addResource(b);
    rm->addResource(c);

    // Partial list: resources not mentioned keep their relative order at the end
    rm->reorderResources({c.id, a.id});

    const QList<WebResource> ordered = rm->getAllResources();
    QCOMPARE(ordered.size(), 3);
    QCOMPARE(ordered[0].id, c.id);
    QCOMPARE(ordered[1].id, a.id);
    QCOMPARE(ordered[2].id, b.id);
    QCOMPARE(ordered[0].order, 0);
    QCOMPARE(ordered[1].order, 1);
    QCOMPARE(ordered[2].order, 2);
}

void TestResourceManager::testStartupResourceFallback() {
    auto *rm = ResourceManager::instance();

    // No resources at all: invalid result
    QVERIFY(!rm->getStartupResource().isValid());

    WebResource a = makeResource("A", "https://a.com");
    WebResource b = makeResource("B", "https://b.com");
    rm->addResource(a);
    rm->addResource(b);

    // LastUsed pointing to a removed/unknown id: falls back to first resource
    rm->setStartupMode(ResourceManager::LastUsed);
    rm->setLastUsedResourceId("gone-id");
    QCOMPARE(rm->getStartupResource().id, a.id);

    // SelectedResource with a valid default: returns exactly that one
    rm->setStartupMode(ResourceManager::SelectedResource);
    rm->setDefaultResourceId(b.id);
    QCOMPARE(rm->getStartupResource().id, b.id);
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

void TestResourceManager::testImportRegeneratesDuplicateIds() {
    auto *rm = ResourceManager::instance();

    WebResource existing = makeResource("Existing", "https://existing.com");
    rm->addResource(existing);

    // Preset file with two entries sharing one id, colliding with `existing`
    QJsonArray arr;
    for (int i = 0; i < 2; ++i) {
        QJsonObject obj;
        obj["id"]         = existing.id; // deliberate collision
        obj["name"]       = QString("Imported %1").arg(i);
        obj["url"]        = QString("https://imported%1.com").arg(i);
        obj["isEnabled"]  = true;
        obj["order"]      = i;
        obj["zoomFactor"] = 1.0;
        arr.append(obj);
    }

    QTemporaryFile tmp;
    tmp.setAutoRemove(true);
    writePresetFile(tmp, arr);

    QVERIFY(rm->importPresets(tmp.fileName()));
    QCOMPARE(rm->getResourceCount(), 3);

    // All ids must be unique after import
    QSet<QString> ids;
    for (const WebResource &r : rm->getAllResources()) {
        ids.insert(r.id);
    }
    QCOMPARE(ids.size(), 3);
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
