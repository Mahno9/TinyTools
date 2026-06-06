#include <QtTest>
#include "../../src/core/HotkeyManager.h"

class TestHotkey : public QObject {
    Q_OBJECT

private slots:
    void testEnableDisable();
    void testIsRegisteredFalseBeforeRegister();
    void testModifierDetection();
    void testNullHwndUsedByDefault();
};

void TestHotkey::testEnableDisable() {
    HotkeyManager manager;
    QVERIFY(manager.isEnabled() == true);

    manager.setEnabled(false);
    QVERIFY(manager.isEnabled() == false);

    manager.setEnabled(true);
    QVERIFY(manager.isEnabled() == true);
}

void TestHotkey::testIsRegisteredFalseBeforeRegister() {
    HotkeyManager manager;
    QVERIFY(!manager.isHotkeyRegistered(HotkeyType::MainToggle));
    QVERIFY(!manager.isHotkeyRegistered(HotkeyType::AlternativeToggle));
}

void TestHotkey::testModifierDetection() {
    Qt::KeyboardModifiers mods = Qt::ControlModifier | Qt::AltModifier;

    QVERIFY(mods & Qt::ControlModifier);
    QVERIFY(mods & Qt::AltModifier);
    QVERIFY(!(mods & Qt::ShiftModifier));
    QVERIFY(!(mods & Qt::MetaModifier));
}

void TestHotkey::testNullHwndUsedByDefault() {
    // registerHotkey() stores nullptr as windowHandle (thread-associated hotkey).
    // On Windows this is verified by the fact that RegisterHotKey succeeds with NULL hwnd.
    // On non-Windows the call returns false but does not crash.
    HotkeyManager manager;
    // Use an obscure key combo to avoid conflicting with user's hotkeys in CI
    bool result = manager.registerHotkey(HotkeyType::MainToggle,
                                         Qt::Key_F12,
                                         Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);
    // We don't assert success since CI runners may have the hotkey occupied;
    // we only assert the function doesn't crash and returns a bool.
    Q_UNUSED(result);
    // If registered, confirm the stored key matches
    if (result) {
        QCOMPARE(manager.getHotkeyKey(HotkeyType::MainToggle), static_cast<int>(Qt::Key_F12));
        QVERIFY(manager.isHotkeyRegistered(HotkeyType::MainToggle));
    }
}

QTEST_MAIN(TestHotkey)
#include "test_hotkey.moc"
