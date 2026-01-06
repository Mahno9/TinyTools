#include "SettingsDialog.h"
#include "../models/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QKeySequenceEdit>
#include <QDebug>

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
    
    m_hotkeyKeySpinBox = new QSpinBox(this);
    m_hotkeyKeySpinBox->setRange(0, 255);
    m_hotkeyKeySpinBox->setToolTip("Virtual key code (e.g., 84 for 'T')");
    hotkeyLayout->addRow("Key Code:", m_hotkeyKeySpinBox);
    
    m_hotkeyModifierCtrl = new QComboBox(this);
    m_hotkeyModifierCtrl->addItem("Disabled", 0);
    m_hotkeyModifierCtrl->addItem("Enabled", 1);
    hotkeyLayout->addRow("Ctrl:", m_hotkeyModifierCtrl);
    
    m_hotkeyModifierAlt = new QComboBox(this);
    m_hotkeyModifierAlt->addItem("Disabled", 0);
    m_hotkeyModifierAlt->addItem("Enabled", 1);
    hotkeyLayout->addRow("Alt:", m_hotkeyModifierAlt);
    
    m_hotkeyModifierShift = new QComboBox(this);
    m_hotkeyModifierShift->addItem("Disabled", 0);
    m_hotkeyModifierShift->addItem("Enabled", 1);
    hotkeyLayout->addRow("Shift:", m_hotkeyModifierShift);
    
    m_hotkeyModifierMeta = new QComboBox(this);
    m_hotkeyModifierMeta->addItem("Disabled", 0);
    m_hotkeyModifierMeta->addItem("Enabled", 1);
    hotkeyLayout->addRow("Win:", m_hotkeyModifierMeta);
    
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
    
    m_languageComboBox = new QComboBox(this);
    m_languageComboBox->addItem("English", "en");
    m_languageComboBox->addItem("Russian", "ru");
    m_languageComboBox->addItem("German", "de");
    m_languageComboBox->addItem("French", "fr");
    m_languageComboBox->addItem("Spanish", "es");
    generalLayout->addRow("Language:", m_languageComboBox);
    
    mainLayout->addWidget(generalGroup);
    
    // Translation Settings Group
    QGroupBox* translationGroup = new QGroupBox("Translation Settings", this);
    QFormLayout* translationLayout = new QFormLayout(translationGroup);
    
    m_autoTranslateCheckBox = new QCheckBox(this);
    m_autoTranslateCheckBox->setToolTip("Automatically translate when clipboard text changes");
    translationLayout->addRow("Auto-translate on Clipboard:", m_autoTranslateCheckBox);
    
    m_sourceLanguageComboBox = new QComboBox(this);
    m_sourceLanguageComboBox->addItem("Auto Detect", "auto");
    m_sourceLanguageComboBox->addItem("English", "en");
    m_sourceLanguageComboBox->addItem("Russian", "ru");
    m_sourceLanguageComboBox->addItem("German", "de");
    m_sourceLanguageComboBox->addItem("French", "fr");
    m_sourceLanguageComboBox->addItem("Spanish", "es");
    m_sourceLanguageComboBox->addItem("Chinese", "zh");
    m_sourceLanguageComboBox->addItem("Japanese", "ja");
    m_sourceLanguageComboBox->addItem("Korean", "ko");
    translationLayout->addRow("Source Language:", m_sourceLanguageComboBox);
    
    m_targetLanguageComboBox = new QComboBox(this);
    m_targetLanguageComboBox->addItem("English", "en");
    m_targetLanguageComboBox->addItem("Russian", "ru");
    m_targetLanguageComboBox->addItem("German", "de");
    m_targetLanguageComboBox->addItem("French", "fr");
    m_targetLanguageComboBox->addItem("Spanish", "es");
    m_targetLanguageComboBox->addItem("Chinese", "zh");
    m_targetLanguageComboBox->addItem("Japanese", "ja");
    m_targetLanguageComboBox->addItem("Korean", "ko");
    translationLayout->addRow("Target Language:", m_targetLanguageComboBox);
    
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
    AppConfig config;
    if (!config.load()) {
        qWarning() << "Failed to load settings";
        return;
    }
    
    // Load hotkey settings
    m_hotkeyKeySpinBox->setValue(config.getHotkeyKey());
    
    Qt::KeyboardModifiers modifiers = config.getHotkeyModifiers();
    m_hotkeyModifierCtrl->setCurrentIndex(modifiers & Qt::ControlModifier ? 1 : 0);
    m_hotkeyModifierAlt->setCurrentIndex(modifiers & Qt::AltModifier ? 1 : 0);
    m_hotkeyModifierShift->setCurrentIndex(modifiers & Qt::ShiftModifier ? 1 : 0);
    m_hotkeyModifierMeta->setCurrentIndex(modifiers & Qt::MetaModifier ? 1 : 0);
    
    // Load window settings
    m_alwaysOnTopCheckBox->setChecked(config.getAlwaysOnTop());
    m_opacitySpinBox->setValue(config.getWindowOpacity());
    
    // Load general settings
    m_autoStartCheckBox->setChecked(config.getAutoStart());
    m_minimizeToTrayCheckBox->setChecked(config.getMinimizeToTray());
    
    QString language = config.getLanguage();
    int langIndex = m_languageComboBox->findData(language);
    if (langIndex >= 0) {
        m_languageComboBox->setCurrentIndex(langIndex);
    }
    
    // Load translation settings
    m_autoTranslateCheckBox->setChecked(config.getAutoTranslate());
    
    QString sourceLanguage = config.getSourceLanguage();
    int sourceLangIndex = m_sourceLanguageComboBox->findData(sourceLanguage);
    if (sourceLangIndex >= 0) {
        m_sourceLanguageComboBox->setCurrentIndex(sourceLangIndex);
    }
    
    QString targetLanguage = config.getTargetLanguage();
    int targetLangIndex = m_targetLanguageComboBox->findData(targetLanguage);
    if (targetLangIndex >= 0) {
        m_targetLanguageComboBox->setCurrentIndex(targetLangIndex);
    }
}

void SettingsDialog::saveSettings() {
    AppConfig config;
    if (!config.load()) {
        qWarning() << "Failed to load config for saving";
        return;
    }
    
    // Save hotkey settings
    int key = m_hotkeyKeySpinBox->value();
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    
    if (m_hotkeyModifierCtrl->currentIndex() == 1) {
        modifiers |= Qt::ControlModifier;
    }
    if (m_hotkeyModifierAlt->currentIndex() == 1) {
        modifiers |= Qt::AltModifier;
    }
    if (m_hotkeyModifierShift->currentIndex() == 1) {
        modifiers |= Qt::ShiftModifier;
    }
    if (m_hotkeyModifierMeta->currentIndex() == 1) {
        modifiers |= Qt::MetaModifier;
    }
    
    config.setHotkey(key, modifiers);
    
    // Save window settings
    config.setAlwaysOnTop(m_alwaysOnTopCheckBox->isChecked());
    config.setWindowOpacity(m_opacitySpinBox->value());
    
    // Save general settings
    config.setAutoStart(m_autoStartCheckBox->isChecked());
    config.setMinimizeToTray(m_minimizeToTrayCheckBox->isChecked());
    
    QString language = m_languageComboBox->currentData().toString();
    config.setLanguage(language);
    
    // Save translation settings
    config.setAutoTranslate(m_autoTranslateCheckBox->isChecked());
    
    QString sourceLanguage = m_sourceLanguageComboBox->currentData().toString();
    config.setSourceLanguage(sourceLanguage);
    
    QString targetLanguage = m_targetLanguageComboBox->currentData().toString();
    config.setTargetLanguage(targetLanguage);
    
    if (!config.save()) {
        qWarning() << "Failed to save settings";
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
    m_hotkeyKeySpinBox->setValue(84); // 'T' key
    m_hotkeyModifierCtrl->setCurrentIndex(1); // Ctrl enabled
    m_hotkeyModifierAlt->setCurrentIndex(1); // Alt enabled
    m_hotkeyModifierShift->setCurrentIndex(0); // Shift disabled
    m_hotkeyModifierMeta->setCurrentIndex(0); // Win disabled
    
    m_alwaysOnTopCheckBox->setChecked(true);
    m_opacitySpinBox->setValue(90);
    
    m_autoStartCheckBox->setChecked(true);
    m_minimizeToTrayCheckBox->setChecked(true);
    
    int langIndex = m_languageComboBox->findData("en");
    if (langIndex >= 0) {
        m_languageComboBox->setCurrentIndex(langIndex);
    }
    
    // Reset translation settings
    m_autoTranslateCheckBox->setChecked(false);
    
    int sourceLangIndex = m_sourceLanguageComboBox->findData("auto");
    if (sourceLangIndex >= 0) {
        m_sourceLanguageComboBox->setCurrentIndex(sourceLangIndex);
    }
    
    int targetLangIndex = m_targetLanguageComboBox->findData("en");
    if (targetLangIndex >= 0) {
        m_targetLanguageComboBox->setCurrentIndex(targetLangIndex);
    }
    
    // Apply the reset settings
    applySettings();
}

void SettingsDialog::onOpacityChanged(int value) {
    Q_UNUSED(value);
    // Could implement live preview of opacity here
    // by temporarily setting the main window opacity
}
