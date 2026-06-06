#include <QtTest>
#include "../../src/models/WebResource.h"

class TestWebResource : public QObject {
    Q_OBJECT

private slots:
    void testValidHttpUrl();
    void testValidHttpsUrl();
    void testRejectsFileUrl();
    void testRejectsJavascriptScheme();
    void testRejectsEmptyUrl();
    void testRejectsNoScheme();
    void testRejectsEmptyId();
    void testRejectsEmptyName();
    void testJsonRoundTrip();
    void testFromJsonGeneratesIdWhenMissing();
    void testCreateFactory();
};

void TestWebResource::testValidHttpUrl() {
    WebResource r = WebResource::create("Test", "http://example.com");
    QVERIFY(r.isValid());
}

void TestWebResource::testValidHttpsUrl() {
    WebResource r = WebResource::create("Test", "https://translate.google.com");
    QVERIFY(r.isValid());
}

void TestWebResource::testRejectsFileUrl() {
    WebResource r = WebResource::create("Test", "file:///C:/Windows/System32");
    QVERIFY(!r.isValid());
}

void TestWebResource::testRejectsJavascriptScheme() {
    WebResource r = WebResource::create("Test", "javascript:alert(1)");
    QVERIFY(!r.isValid());
}

void TestWebResource::testRejectsEmptyUrl() {
    WebResource r = WebResource::create("Test", "");
    QVERIFY(!r.isValid());
}

void TestWebResource::testRejectsNoScheme() {
    // "example.com" has no scheme — QUrl scheme() returns ""
    WebResource r = WebResource::create("Test", "example.com");
    QVERIFY(!r.isValid());
}

void TestWebResource::testRejectsEmptyId() {
    WebResource r;
    r.name = "Name";
    r.url  = "https://example.com";
    // id is empty → invalid
    QVERIFY(!r.isValid());
}

void TestWebResource::testRejectsEmptyName() {
    WebResource r;
    r.id  = "some-uuid";
    r.url = "https://example.com";
    QVERIFY(!r.isValid());
}

void TestWebResource::testJsonRoundTrip() {
    WebResource original = WebResource::create("Google Translate", "https://translate.google.com");
    original.initScript    = "console.log('init');";
    original.openScript    = "document.querySelector('textarea').focus();";
    original.altOpenScript = "document.querySelector('textarea').value = window.tinyToolsClipboard;";
    original.isEnabled     = true;
    original.order         = 3;
    original.zoomFactor    = 1.25;

    QJsonObject json = original.toJson();
    WebResource restored = WebResource::fromJson(json);

    QCOMPARE(restored.id,            original.id);
    QCOMPARE(restored.name,          original.name);
    QCOMPARE(restored.url,           original.url);
    QCOMPARE(restored.initScript,    original.initScript);
    QCOMPARE(restored.openScript,    original.openScript);
    QCOMPARE(restored.altOpenScript, original.altOpenScript);
    QCOMPARE(restored.isEnabled,     original.isEnabled);
    QCOMPARE(restored.order,         original.order);
    QCOMPARE(restored.zoomFactor,    original.zoomFactor);
    QVERIFY(restored.isValid());
}

void TestWebResource::testFromJsonGeneratesIdWhenMissing() {
    QJsonObject json;
    json["name"]      = "No ID";
    json["url"]       = "https://example.com";
    json["isEnabled"] = true;
    json["order"]     = 0;
    json["zoomFactor"] = 1.0;
    // "id" key intentionally absent

    WebResource r = WebResource::fromJson(json);
    QVERIFY(!r.id.isEmpty());  // auto-generated UUID
}

void TestWebResource::testCreateFactory() {
    WebResource r = WebResource::create("Name", "https://example.com");
    QVERIFY(!r.id.isEmpty());
    QCOMPARE(r.name, QString("Name"));
    QCOMPARE(r.url,  QString("https://example.com"));
    QVERIFY(r.isEnabled);
    QCOMPARE(r.order,      0);
    QCOMPARE(r.zoomFactor, 1.0);
}

QTEST_MAIN(TestWebResource)
#include "test_webresource.moc"
