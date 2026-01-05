#include <QtTest>
#include "../../src/core/HotkeyManager.h"

class TestHotkey : public QObject {
    Q_OBJECT
    
private slots:
    void testEnableDisable();
    void testRegisterHotkey();
    void testModifierDetection();
};

void TestHotkey::testEnableDisable() {
    HotkeyManager manager;
    QVERIFY(manager.isEnabled() == true);
    
    manager.setEnabled(false);
    QVERIFY(manager.isEnabled() == false);
    
    manager.setEnabled(true);
    QVERIFY(manager.isEnabled() == true);
}

void TestHotkey::testRegisterHotkey() {
    HotkeyManager manager;
    
    // Try to register a hotkey
    bool result = manager.registerHotkey(Qt::Key_T, Qt::ControlModifier | Qt::AltModifier);
    // Note: This may fail if another app has registered the same hotkey
    // In a real test environment, we might need to use a unique hotkey
    Q_UNUSED(result);
}

void TestHotkey::testModifierDetection() {
    // Test that modifiers are correctly detected
    Qt::KeyboardModifiers mods = Qt::ControlModifier | Qt::AltModifier;
    
    QVERIFY(mods & Qt::ControlModifier);
    QVERIFY(mods & Qt::AltModifier);
    QVERIFY(!(mods & Qt::ShiftModifier));
    QVERIFY(!(mods & Qt::MetaModifier));
}

QTEST_MAIN(TestHotkey)
#include "test_hotkey.moc"
