#include "SettingsDialog.h"
#include "../models/AppConfig.h"
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
    
    m_autoStartCheckBox = new QCheckBox(this);
    m_autoStartCheckBox->setToolTip("Start application automatically on Windows login");
    generalLayout->addRow("Auto-start on Login:", m_autoStartCheckBox);
    
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
    int hotkeyKey = config->getHotkeyKey();
    qDebug() << "Hotkey key from config:" << hotkeyKey;
    m_hotkeyKeyLineEdit->setText(QString::number(hotkeyKey));
    
    Qt::KeyboardModifiers modifiers = config->getHotkeyModifiers();
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
    qDebug() << "SettingsDialog::loadSettings() - EXIT";
    
    // Load window settings
    m_alwaysOnTopCheckBox->setChecked(config->getAlwaysOnTop());
    m_opacitySpinBox->setValue(config->getWindowOpacity());
    
    // Load general settings
    m_autoStartCheckBox->setChecked(config->getAutoStart());
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
    config->setHotkey(key, modifiers);
    qDebug() << "Hotkey settings configured";
    
    // Save window settings
    config->setAlwaysOnTop(m_alwaysOnTopCheckBox->isChecked());
    config->setWindowOpacity(m_opacitySpinBox->value());
    qDebug() << "Window settings configured";
    
    // Save general settings
    config->setAutoStart(m_autoStartCheckBox->isChecked());
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
    
    m_autoStartCheckBox->setChecked(true);
    m_minimizeToTrayCheckBox->setChecked(true);
    m_darkThemeCheckBox->setChecked(false);
    
    // Reset translation settings
    m_autoTranslateCheckBox->setChecked(false);
    
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
