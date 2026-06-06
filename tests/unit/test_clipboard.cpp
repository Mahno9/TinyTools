#include <QtTest>
#include <QClipboard>
#include <QGuiApplication>
#include "../../src/core/ClipboardManager.h"

class TestClipboard : public QObject {
    Q_OBJECT
    
private slots:
    void testGetEmptyText();
    void testSetText();
    void testIsValidText();
    void testTrimText();
};

void TestClipboard::testGetEmptyText() {
    QGuiApplication::clipboard()->clear();
    ClipboardManager manager;
    QVERIFY(manager.getText().isEmpty());
}

void TestClipboard::testSetText() {
    ClipboardManager manager;
    manager.setText("Test text");
    QString text = manager.getText();
    // Note: ClipboardManager filters text, so we check if it contains our text
    QVERIFY(text.contains("Test text"));
}

void TestClipboard::testIsValidText() {
    QVERIFY(ClipboardManager::isValidText("Valid text"));
    QVERIFY(!ClipboardManager::isValidText(""));
    QVERIFY(!ClipboardManager::isValidText("   "));
    QVERIFY(!ClipboardManager::isValidText(QString().fill('a', 100001)));
}

void TestClipboard::testTrimText() {
    QString shortText = "Short text";
    QCOMPARE(ClipboardManager::trimText(shortText, 20), shortText);
    
    QString longText = QString().fill('a', 100);
    QString trimmed = ClipboardManager::trimText(longText, 50);
    QCOMPARE(trimmed.length(), 53); // 50 chars + "..."
    QVERIFY(trimmed.endsWith("..."));
}

QTEST_MAIN(TestClipboard)
#include "test_clipboard.moc"
