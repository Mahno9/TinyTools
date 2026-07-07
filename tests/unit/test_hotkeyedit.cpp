#include <QtTest>
#include "../../src/ui/HotkeyEdit.h"

class TestHotkeyEdit : public QObject {
    Q_OBJECT

private slots:
    void testSetHotkeyFormatsString();
    void testParseSimpleCombo();
    void testParseNamedKeys();
    void testParseWinModifierAliases();
    void testParseIsCaseInsensitive();
    void testRoundTrip();
    void testEmptyAndInvalidInput();
};

void TestHotkeyEdit::testSetHotkeyFormatsString() {
    HotkeyEdit edit;
    edit.setHotkey(Qt::Key_T, Qt::ControlModifier | Qt::AltModifier);
    QCOMPARE(edit.hotkeyString(), QString("Ctrl+Alt+T"));

    edit.setHotkey(Qt::Key_F5, Qt::ShiftModifier);
    QCOMPARE(edit.hotkeyString(), QString("Shift+F5"));

    edit.setHotkey(Qt::Key_Space, Qt::MetaModifier);
    QCOMPARE(edit.hotkeyString(), QString("Win+Space"));
}

void TestHotkeyEdit::testParseSimpleCombo() {
    HotkeyEdit edit;
    edit.setHotkeyString("Ctrl+Alt+T");
    QCOMPARE(edit.key(), static_cast<int>(Qt::Key_T));
    QCOMPARE(edit.modifiers(), Qt::ControlModifier | Qt::AltModifier);
}

void TestHotkeyEdit::testParseNamedKeys() {
    HotkeyEdit edit;

    edit.setHotkeyString("Ctrl+Space");
    QCOMPARE(edit.key(), static_cast<int>(Qt::Key_Space));

    edit.setHotkeyString("Ctrl+PageDown");
    QCOMPARE(edit.key(), static_cast<int>(Qt::Key_PageDown));

    edit.setHotkeyString("Alt+F12");
    QCOMPARE(edit.key(), static_cast<int>(Qt::Key_F12));
    QCOMPARE(edit.modifiers(), Qt::AltModifier);

    edit.setHotkeyString("Ctrl+Del");
    QCOMPARE(edit.key(), static_cast<int>(Qt::Key_Delete));
}

void TestHotkeyEdit::testParseWinModifierAliases() {
    HotkeyEdit edit;
    for (const QString &alias : {QString("Win"), QString("Meta"), QString("Super")}) {
        edit.setHotkeyString(alias + "+Z");
        QCOMPARE(edit.modifiers(), Qt::MetaModifier);
        QCOMPARE(edit.key(), static_cast<int>(Qt::Key_Z));
    }
}

void TestHotkeyEdit::testParseIsCaseInsensitive() {
    HotkeyEdit edit;
    edit.setHotkeyString("cTrL+aLt+t");
    QCOMPARE(edit.key(), static_cast<int>(Qt::Key_T));
    QCOMPARE(edit.modifiers(), Qt::ControlModifier | Qt::AltModifier);
}

void TestHotkeyEdit::testRoundTrip() {
    HotkeyEdit source;
    HotkeyEdit target;

    const QList<QPair<int, Qt::KeyboardModifiers>> cases = {
        {Qt::Key_T, Qt::ControlModifier | Qt::AltModifier},
        {Qt::Key_F9, Qt::ControlModifier | Qt::ShiftModifier | Qt::MetaModifier},
        {Qt::Key_9, Qt::AltModifier},
        {Qt::Key_Home, Qt::ControlModifier},
    };

    for (const auto &c : cases) {
        source.setHotkey(c.first, c.second);
        target.setHotkeyString(source.hotkeyString());
        QCOMPARE(target.key(), c.first);
        QCOMPARE(target.modifiers(), c.second);
    }
}

void TestHotkeyEdit::testEmptyAndInvalidInput() {
    HotkeyEdit edit;

    edit.setHotkeyString("");
    QCOMPARE(edit.key(), 0);
    QCOMPARE(edit.modifiers(), Qt::NoModifier);
    QVERIFY(edit.hotkeyString().isEmpty());

    // Modifiers only, no key
    edit.setHotkeyString("Ctrl+");
    QCOMPARE(edit.key(), 0);
    QCOMPARE(edit.modifiers(), Qt::ControlModifier);

    // Unknown key name yields no key
    edit.setHotkeyString("Ctrl+NoSuchKey");
    QCOMPARE(edit.key(), 0);
}

QTEST_MAIN(TestHotkeyEdit)
#include "test_hotkeyedit.moc"
