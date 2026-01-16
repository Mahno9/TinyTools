#include "SettingsDialog.h"
#include "../models/AppConfig.h"
#include "../core/HotkeyManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QKeySequenceEdit>
#include <QDebug>
#include <QValidator>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    setMinimumWidth(500);
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Hotkey Settings Group
    QGroupBox* hotkeyGroup = new QGroupBox("Hotkey Settings", this);
    QFormLayout* hotkeyLayout = new QFormLayout(hotkeyGroup);
    
    m_hotkeyKeyLineEdit = new QLineEdit(this);
    m_hotkeyKeyLineEdit->setToolTip("Enter virtual key code (e.g., 84 for 'T') or character (e.g., 'T', 'A', '1')");
    // Removed restrictive input mask to allow flexible input (both numbers and single characters)
    m_hotkeyKeyLineEdit->setMaxLength(3);
    m_hotkeyKeyLineEdit->setPlaceholderText("84 or T");
    hotkeyLayout->addRow("Key Code:", m_hotkeyKeyLineEdit);
    
    QLabel* keyDescriptionLabel = new QLabel("Enter virtual key code (e.g., 84 for 'T') or character (e.g., 'T', 'A', '1')", this);
    keyDescriptionLabel->setWordWrap(true);
    keyDescriptionLabel->setStyleSheet("color: gray; font-size: 10px;");
    hotkeyLayout->addRow("", keyDescriptionLabel);
    
    m_hotkeyModifierCtrl = new QCheckBox(this);
    m_hotkeyModifierCtrl->setToolTip("Enable Ctrl modifier");
    hotkeyLayout->addRow("Ctrl:", m_hotkeyModifierCtrl);
    
    m_hotkeyModifierAlt = new QCheckBox(this);
    m_hotkeyModifierAlt->setToolTip("Enable Alt modifier");
    hotkeyLayout->addRow("Alt:", m_hotkeyModifierAlt);
    
    m_hotkeyModifierShift = new QCheckBox(this);
    m_hotkeyModifierShift->setToolTip("Enable Shift modifier");
    hotkeyLayout->addRow("Shift:", m_hotkeyModifierShift);
    
    mainLayout->addWidget(hotkeyGroup);
    
    // Show and Translate Hotkey Group
    QGroupBox* showTranslateGroup = new QGroupBox("Show and Translate Hotkey", this);
    QFormLayout* showTranslateLayout = new QFormLayout(showTranslateGroup);
    
    showTranslateLayout->addRow(new QLabel("This hotkey opens the window and translates clipboard content.", this));
    
    m_showTranslateKeyLineEdit = new QLineEdit(this);
    m_showTranslateKeyLineEdit->setToolTip("Enter virtual key code (e.g., 83 for 'S') or character (e.g., 'S', 'A', '1')");
    m_showTranslateKeyLineEdit->setMaxLength(3);
    m_showTranslateKeyLineEdit->setPlaceholderText("83 or S");
    showTranslateLayout->addRow("Key Code:", m_showTranslateKeyLineEdit);
    
    QLabel* showTranslateKeyDescriptionLabel = new QLabel("Enter virtual key code (e.g., 83 for 'S') or character (e.g., 'S', 'A', '1')", this);
    showTranslateKeyDescriptionLabel->setWordWrap(true);
    showTranslateKeyDescriptionLabel->setStyleSheet("color: gray; font-size: 10px;");
    showTranslateLayout->addRow("", showTranslateKeyDescriptionLabel);
    
    m_showTranslateModifierCtrl = new QCheckBox(this);
    m_showTranslateModifierCtrl->setToolTip("Enable Ctrl modifier");
    showTranslateLayout->addRow("Ctrl:", m_showTranslateModifierCtrl);
    
    m_showTranslateModifierAlt = new QCheckBox(this);
    m_showTranslateModifierAlt->setToolTip("Enable Alt modifier");
    showTranslateLayout->addRow("Alt:", m_showTranslateModifierAlt);
    
    m_showTranslateModifierShift = new QCheckBox(this);
    m_showTranslateModifierShift->setToolTip("Enable Shift modifier");
    showTranslateLayout->addRow("Shift:", m_showTranslateModifierShift);
    
    mainLayout->addWidget(showTranslateGroup);
    
    // Window Settings Group
    QGroupBox* windowGroup = new QGroupBox("Window Settings", this);
    QFormLayout* windowLayout = new QFormLayout(windowGroup);
    
    m_alwaysOnTopCheckBox = new QCheckBox(this);
    m_alwaysOnTopCheckBox->setToolTip("Keep window always on top of other windows");
    windowLayout->addRow("Always on Top:", m_alwaysOnTopCheckBox);
    
    m_opacitySpinBox = new QSpinBox(this);
    m_opacitySpinBox->setRange(20, 100);
    m_opacitySpinBox->setSuffix("%");
    m_opacitySpinBox->setToolTip("Window transparency (20-100%)");
    windowLayout->addRow("Opacity:", m_opacitySpinBox);
    
    mainLayout->addWidget(windowGroup);
    
    // General Settings Group
    QGroupBox* generalGroup = new QGroupBox("General Settings", this);
    QFormLayout* generalLayout = new QFormLayout(generalGroup);
    
    m_autoStartOnLoginCheckBox = new QCheckBox(this);
    m_autoStartOnLoginCheckBox->setToolTip("Add application to Windows startup programs to launch automatically when you log in");
    generalLayout->addRow("Auto-start on Login:", m_autoStartOnLoginCheckBox);
    
    m_showWindowOnStartupCheckBox = new QCheckBox(this);
    m_showWindowOnStartupCheckBox->setToolTip("Show the application window when it starts (if disabled, runs in background)");
    generalLayout->addRow("Show Window on Startup:", m_showWindowOnStartupCheckBox);
    
    m_minimizeToTrayCheckBox = new QCheckBox(this);
    m_minimizeToTrayCheckBox->setToolTip("Minimize to system tray instead of closing");
    generalLayout->addRow("Minimize to Tray:", m_minimizeToTrayCheckBox);
    
    m_darkThemeCheckBox = new QCheckBox(this);
    m_darkThemeCheckBox->setToolTip("Enable dark theme for the application");
    generalLayout->addRow("Dark Theme:", m_darkThemeCheckBox);
    
    mainLayout->addWidget(generalGroup);
    
    // Translation Settings Group
    QGroupBox* translationGroup = new QGroupBox("Translation Settings", this);
    QFormLayout* translationLayout = new QFormLayout(translationGroup);
    
    m_autoTranslateCheckBox = new QCheckBox(this);
    m_autoTranslateCheckBox->setToolTip("Automatically translate when clipboard text changes");
    translationLayout->addRow("Auto-translate on Clipboard:", m_autoTranslateCheckBox);
    
    mainLayout->addWidget(translationGroup);
    
    // Button Box
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton("Reset to Defaults", this);
    m_resetButton->setToolTip("Reset all settings to default values");
    buttonLayout->addWidget(m_resetButton);
    
    buttonLayout->addStretch();
    
    m_applyButton = new QPushButton("Apply", this);
    buttonLayout->addWidget(m_applyButton);
    
    QPushButton* okButton = new QPushButton("OK", this);
    buttonLayout->addWidget(okButton);
    
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::applySettings);
    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onAccepted);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(m_opacitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::onOpacityChanged);
}

void SettingsDialog::loadSettings() {
    qDebug() << "SettingsDialog::loadSettings() - ENTRY";
    
    AppConfig* config = AppConfig::instance();
    if (!config->load()) {
        qWarning() << "Failed to load settings";
        qDebug() << "SettingsDialog::loadSettings() - EXIT (failed to load)";
        return;
    }
    
    qDebug() << "Settings loaded successfully from config";
    
    // Load hotkey settings
    int hotkeyKey = config->getHotkeyKey(HotkeyType::MainToggle);
    qDebug() << "Hotkey key from config:" << hotkeyKey;
    m_hotkeyKeyLineEdit->setText(QString::number(hotkeyKey));
    
    Qt::KeyboardModifiers modifiers = config->getHotkeyModifiers(HotkeyType::MainToggle);
    qDebug() << "=== LOADING HOTKEY MODIFIERS ===";
    qDebug() << "Modifiers raw value from config:" << static_cast<int>(modifiers);
    
    bool ctrlEnabled = (modifiers & Qt::ControlModifier);
    bool altEnabled = (modifiers & Qt::AltModifier);
    bool shiftEnabled = (modifiers & Qt::ShiftModifier);
    
    qDebug() << "  Ctrl modifier from config:" << (ctrlEnabled ? "YES" : "NO");
    qDebug() << "  Alt modifier from config:" << (altEnabled ? "YES" : "NO");
    qDebug() << "  Shift modifier from config:" << (shiftEnabled ? "YES" : "NO");
    
    m_hotkeyModifierCtrl->setChecked(ctrlEnabled);
    m_hotkeyModifierAlt->setChecked(altEnabled);
    m_hotkeyModifierShift->setChecked(shiftEnabled);
    
    qDebug() << "Checkbox states set to match config";
    
    // Load show and translate hotkey settings
    int showTranslateKey = config->getHotkeyKey(HotkeyType::ShowAndTranslate);
    qDebug() << "Show and Translate hotkey key from config:" << showTranslateKey;
    m_showTranslateKeyLineEdit->setText(QString::number(showTranslateKey));
    
    Qt::KeyboardModifiers showTranslateModifiers = config->getHotkeyModifiers(HotkeyType::ShowAndTranslate);
    qDebug() << "=== LOADING SHOW AND TRANSLATE HOTKEY MODIFIERS ===";
    qDebug() << "Modifiers raw value from config:" << static_cast<int>(showTranslateModifiers);
    
    bool showTranslateCtrlEnabled = (showTranslateModifiers & Qt::ControlModifier);
    bool showTranslateAltEnabled = (showTranslateModifiers & Qt::AltModifier);
    bool showTranslateShiftEnabled = (showTranslateModifiers & Qt::ShiftModifier);
    
    qDebug() << "  Show and Translate Ctrl modifier from config:" << (showTranslateCtrlEnabled ? "YES" : "NO");
    qDebug() << "  Show and Translate Alt modifier from config:" << (showTranslateAltEnabled ? "YES" : "NO");
    qDebug() << "  Show and Translate Shift modifier from config:" << (showTranslateShiftEnabled ? "YES" : "NO");
    
    m_showTranslateModifierCtrl->setChecked(showTranslateCtrlEnabled);
    m_showTranslateModifierAlt->setChecked(showTranslateAltEnabled);
    m_showTranslateModifierShift->setChecked(showTranslateShiftEnabled);
    
    qDebug() << "Show and Translate checkbox states set to match config";
    qDebug() << "SettingsDialog::loadSettings() - EXIT";
    
    // Load window settings
    m_alwaysOnTopCheckBox->setChecked(config->getAlwaysOnTop());
    m_opacitySpinBox->setValue(config->getWindowOpacity());
    
    // Load general settings
    m_autoStartOnLoginCheckBox->setChecked(config->getAutoStartOnLogin());
    m_showWindowOnStartupCheckBox->setChecked(config->getShowWindowOnStartup());
    m_minimizeToTrayCheckBox->setChecked(config->getMinimizeToTray());
    m_darkThemeCheckBox->setChecked(config->getDarkTheme());
    
    // Load translation settings
    m_autoTranslateCheckBox->setChecked(config->getAutoTranslate());
}

void SettingsDialog::saveSettings() {
    qDebug() << "SettingsDialog::saveSettings() - Starting save operation";
    
    AppConfig* config = AppConfig::instance();
    
    // Save hotkey settings
    QString keyText = m_hotkeyKeyLineEdit->text().trimmed();
    qDebug() << "Key text from UI:" << keyText;
    int key = stringToKeyCode(keyText);
    
    if (key == -1) {
        qWarning() << "Invalid hotkey key code:" << keyText << "- Using default hotkey";
        key = static_cast<int>(Qt::Key_T); // Use default 'T' key
    }
    qDebug() << "Resolved key code:" << key;
    
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    
    qDebug() << "=== BUILDING HOTKEY MODIFIERS FROM CHECKBOXES ===";
    bool ctrlChecked = m_hotkeyModifierCtrl->isChecked();
    bool altChecked = m_hotkeyModifierAlt->isChecked();
    bool shiftChecked = m_hotkeyModifierShift->isChecked();
    
    qDebug() << "  Ctrl checkbox state:" << (ctrlChecked ? "CHECKED" : "UNCHECKED");
    qDebug() << "  Alt checkbox state:" << (altChecked ? "CHECKED" : "UNCHECKED");
    qDebug() << "  Shift checkbox state:" << (shiftChecked ? "CHECKED" : "UNCHECKED");
    
    if (ctrlChecked) {
        modifiers |= Qt::ControlModifier;
        qDebug() << "  Added Qt::ControlModifier";
    }
    if (altChecked) {
        modifiers |= Qt::AltModifier;
        qDebug() << "  Added Qt::AltModifier";
    }
    if (shiftChecked) {
        modifiers |= Qt::ShiftModifier;
        qDebug() << "  Added Qt::ShiftModifier";
    }
    
    qDebug() << "Final modifiers value:" << static_cast<int>(modifiers);
    qDebug() << "Calling config->setHotkey()...";
    config->setHotkey(HotkeyType::MainToggle, key, modifiers);
    qDebug() << "Hotkey settings configured";
    
    // Save show and translate hotkey settings
    QString showTranslateKeyText = m_showTranslateKeyLineEdit->text().trimmed();
    qDebug() << "Show and Translate key text from UI:" << showTranslateKeyText;
    int showTranslateKey = stringToKeyCode(showTranslateKeyText);
    
    if (showTranslateKey == -1) {
        qWarning() << "Invalid show and translate hotkey key code:" << showTranslateKeyText << "- Using default hotkey";
        showTranslateKey = static_cast<int>(Qt::Key_S); // Use default 'S' key
    }
    qDebug() << "Resolved show and translate key code:" << showTranslateKey;
    
    Qt::KeyboardModifiers showTranslateModifiers = Qt::NoModifier;
    
    qDebug() << "=== BUILDING SHOW AND TRANSLATE HOTKEY MODIFIERS FROM CHECKBOXES ===";
    bool showTranslateCtrlChecked = m_showTranslateModifierCtrl->isChecked();
    bool showTranslateAltChecked = m_showTranslateModifierAlt->isChecked();
    bool showTranslateShiftChecked = m_showTranslateModifierShift->isChecked();
    
    qDebug() << "  Show and Translate Ctrl checkbox state:" << (showTranslateCtrlChecked ? "CHECKED" : "UNCHECKED");
    qDebug() << "  Show and Translate Alt checkbox state:" << (showTranslateAltChecked ? "CHECKED" : "UNCHECKED");
    qDebug() << "  Show and Translate Shift checkbox state:" << (showTranslateShiftChecked ? "CHECKED" : "UNCHECKED");
    
    if (showTranslateCtrlChecked) {
        showTranslateModifiers |= Qt::ControlModifier;
        qDebug() << "  Added Qt::ControlModifier";
    }
    if (showTranslateAltChecked) {
        showTranslateModifiers |= Qt::AltModifier;
        qDebug() << "  Added Qt::AltModifier";
    }
    if (showTranslateShiftChecked) {
        showTranslateModifiers |= Qt::ShiftModifier;
        qDebug() << "  Added Qt::ShiftModifier";
    }
    
    qDebug() << "Final show and translate modifiers value:" << static_cast<int>(showTranslateModifiers);
    qDebug() << "Calling config->setHotkey()...";
    config->setHotkey(HotkeyType::ShowAndTranslate, showTranslateKey, showTranslateModifiers);
    qDebug() << "Show and Translate hotkey settings configured";
    
    // Save window settings
    config->setAlwaysOnTop(m_alwaysOnTopCheckBox->isChecked());
    config->setWindowOpacity(m_opacitySpinBox->value());
    qDebug() << "Window settings configured";
    
    // Save general settings
    config->setAutoStartOnLogin(m_autoStartOnLoginCheckBox->isChecked());
    config->setShowWindowOnStartup(m_showWindowOnStartupCheckBox->isChecked());
    config->setMinimizeToTray(m_minimizeToTrayCheckBox->isChecked());
    config->setDarkTheme(m_darkThemeCheckBox->isChecked());
    qDebug() << "General settings configured";
    
    // Save translation settings
    config->setAutoTranslate(m_autoTranslateCheckBox->isChecked());
    qDebug() << "Translation settings configured";
    
    qDebug() << "Calling config->save()...";
    if (!config->save()) {
        qCritical() << "Failed to save settings to disk";
        QMessageBox::critical(
            this,
            "Save Failed",
            QString("Failed to save settings to:\n%1\n\nPlease check if you have write permissions.")
                .arg(config->getConfigFilePath())
        );
        qDebug() << "SettingsDialog::saveSettings() - Completed with errors";
    } else {
        qInfo() << "Settings saved successfully";
    }
}

void SettingsDialog::applySettings() {
    saveSettings();
}

void SettingsDialog::onAccepted() {
    saveSettings();
    accept();
}

void SettingsDialog::onResetClicked() {
    // Reset all UI elements to defaults
    m_hotkeyKeyLineEdit->setText("84"); // 'T' key
    m_hotkeyModifierCtrl->setChecked(true); // Ctrl enabled
    m_hotkeyModifierAlt->setChecked(true); // Alt enabled
    m_hotkeyModifierShift->setChecked(false); // Shift disabled
    
    m_alwaysOnTopCheckBox->setChecked(true);
    m_opacitySpinBox->setValue(90);
    
    m_autoStartOnLoginCheckBox->setChecked(false);
    m_showWindowOnStartupCheckBox->setChecked(true);
    m_minimizeToTrayCheckBox->setChecked(true);
    m_darkThemeCheckBox->setChecked(false);
    
    // Reset translation settings
    m_autoTranslateCheckBox->setChecked(false);
    
    // Reset show and translate hotkey to defaults
    m_showTranslateKeyLineEdit->setText("83"); // 'S' key
    m_showTranslateModifierCtrl->setChecked(true); // Ctrl enabled
    m_showTranslateModifierAlt->setChecked(true); // Alt enabled
    m_showTranslateModifierShift->setChecked(false); // Shift disabled
    
    // Apply the reset settings
    applySettings();
}

void SettingsDialog::onOpacityChanged(int value) {
    Q_UNUSED(value);
    // Could implement live preview of opacity here
    // by temporarily setting the main window opacity
}

int SettingsDialog::stringToKeyCode(const QString& text) {
    if (text.isEmpty()) {
        return -1;
    }
    
    // Try to parse as number
    bool ok;
    int keyCode = text.toInt(&ok);
    if (ok && keyCode >= 0 && keyCode <= 255) {
        return keyCode;
    }
    
    // If not a number, try to interpret as a single character
    if (text.length() == 1) {
        QChar ch = text.at(0).toUpper();
        
        // Map characters to virtual key codes
        // For A-Z: 65 is 'A', 90 is 'Z'
        if (ch >= 'A' && ch <= 'Z') {
            return ch.unicode();
        }
        
        // For 0-9: 48 is '0', 57 is '9'
        if (ch >= '0' && ch <= '9') {
            return ch.unicode();
        }
        
        // Space
        if (ch == ' ') {
            return 32;
        }
        
        // Return key
        if (ch == '\r' || ch == '\n') {
            return 13;
        }
        
        // Tab
        if (ch == '\t') {
            return 9;
        }
        
        // Escape
        if (ch == '\x1b') {
            return 27;
        }
        
        qWarning() << "Unsupported character for hotkey:" << ch;
        return -1;
    }
    
    qWarning() << "Invalid key code format:" << text;
    return -1;
}
