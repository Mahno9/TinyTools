#include "SettingsDialog.h"
#include "../core/HotkeyManager.h"
#include "../models/AppConfig.h"
#include "../models/ResourceManager.h"
#include "../models/WebResource.h"
#ifdef Q_OS_WIN
#include <dwmapi.h>
#endif
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("TinyTools Settings");
  setMinimumSize(600, 600);
  setupUI();
  loadSettings();
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::setupUI() {
  // Apply theme based on config
  bool darkTheme = AppConfig::instance()->getDarkTheme();
  if (darkTheme) {
#ifdef Q_OS_WIN
    BOOL useDarkMode = TRUE;
    // DWMWA_USE_IMMERSIVE_DARK_MODE is 20
    DwmSetWindowAttribute(reinterpret_cast<HWND>(winId()), 20, &useDarkMode,
                          sizeof(useDarkMode));
#endif
    setStyleSheet(
        "QDialog { background-color: #2b2b2b; color: #ddd; }"
        "QTabWidget::pane { border: 1px solid #444; background: #333; }"
        "QTabBar::tab { background: #3b3b3b; color: #aaa; padding: 8px 16px; "
        "border: none; }"
        "QTabBar::tab:selected { background: #555; color: #fff; }"
        "QTabBar::tab:hover { background: #444; }"
        "QGroupBox { border: 1px solid #555; margin-top: 10px; padding-top: "
        "10px; color: #ccc; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
        "5px; }"
        "QLabel { color: #ccc; }"
        "QLineEdit, QSpinBox, QComboBox, QPlainTextEdit { background: #3b3b3b; "
        "color: #ddd; border: 1px solid #555; padding: 4px; }"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, "
        "QPlainTextEdit:focus { border-color: #0078d7; }"
        "QPushButton { background: #444; color: #ddd; border: 1px solid #555; "
        "padding: 6px 12px; }"
        "QPushButton:hover { background: #555; }"
        "QPushButton:pressed { background: #333; }"
        "QCheckBox { color: #ccc; }"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: #333; width: 12px; }"
        "QScrollBar::handle:vertical { background: #555; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "height: 0; }");
  } else {
    // Light theme (default)
    setStyleSheet(
        "QDialog { background-color: #f5f5f5; color: #333; }"
        "QTabWidget::pane { border: 1px solid #ccc; background: #fff; }"
        "QTabBar::tab { background: #e0e0e0; color: #555; padding: 8px 16px; "
        "border: none; }"
        "QTabBar::tab:selected { background: #fff; color: #000; }"
        "QTabBar::tab:hover { background: #d0d0d0; }"
        "QGroupBox { border: 1px solid #ccc; margin-top: 10px; padding-top: "
        "10px; color: #333; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
        "5px; }"
        "QLabel { color: #333; }"
        "QLineEdit, QSpinBox, QComboBox, QPlainTextEdit { background: #fff; "
        "color: #333; border: 1px solid #ccc; padding: 4px; }"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, "
        "QPlainTextEdit:focus { border-color: #0078d7; }"
        "QPushButton { background: #e0e0e0; color: #333; border: 1px solid "
        "#ccc; padding: 6px 12px; }"
        "QPushButton:hover { background: #d0d0d0; }"
        "QPushButton:pressed { background: #c0c0c0; }"
        "QCheckBox { color: #333; }"
        "QScrollArea { background: transparent; border: none; }");
  }

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Create tab widget
  m_tabWidget = new QTabWidget(this);

  // Tab 1: General
  QWidget *generalTab = new QWidget();
  setupGeneralTab(generalTab);
  m_tabWidget->addTab(generalTab, "General");

  // Tab 2: Resources
  QWidget *resourcesTab = new QWidget();
  setupResourcesTab(resourcesTab);
  m_tabWidget->addTab(resourcesTab, "Resources");

  mainLayout->addWidget(m_tabWidget);

  // Button Box (common for both tabs)
  QHBoxLayout *buttonLayout = new QHBoxLayout();

  m_resetButton = new QPushButton("Reset to Defaults", this);
  m_resetButton->setToolTip("Reset all settings to default values");
  buttonLayout->addWidget(m_resetButton);

  buttonLayout->addStretch();

  m_applyButton = new QPushButton("Apply", this);
  buttonLayout->addWidget(m_applyButton);

  QPushButton *okButton = new QPushButton("OK", this);
  buttonLayout->addWidget(okButton);

  QPushButton *cancelButton = new QPushButton("Cancel", this);
  buttonLayout->addWidget(cancelButton);

  mainLayout->addLayout(buttonLayout);

  // Connect common signals
  connect(m_resetButton, &QPushButton::clicked, this,
          &SettingsDialog::onResetClicked);
  connect(m_applyButton, &QPushButton::clicked, this,
          &SettingsDialog::applySettings);
  connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onAccepted);
  connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::setupGeneralTab(QWidget *tab) {
  QVBoxLayout *layout = new QVBoxLayout(tab);

  // === Main Toggle Hotkey Group ===
  QGroupBox *mainHotkeyGroup = new QGroupBox("Main Open Hotkey", tab);
  QFormLayout *mainHotkeyLayout = new QFormLayout(mainHotkeyGroup);

  mainHotkeyLayout->addRow(
      new QLabel("Open window and execute Main Script", this));

  m_hotkeyKeyLineEdit = new QLineEdit(this);
  m_hotkeyKeyLineEdit->setToolTip(
      "Enter key code (e.g., 84 for 'T') or character");
  m_hotkeyKeyLineEdit->setMaxLength(3);
  m_hotkeyKeyLineEdit->setPlaceholderText("84 or T");
  mainHotkeyLayout->addRow("Key:", m_hotkeyKeyLineEdit);

  m_hotkeyModifierCtrl = new QCheckBox("Ctrl", this);
  m_hotkeyModifierAlt = new QCheckBox("Alt", this);
  m_hotkeyModifierShift = new QCheckBox("Shift", this);

  QHBoxLayout *modifiersLayout = new QHBoxLayout();
  modifiersLayout->addWidget(m_hotkeyModifierCtrl);
  modifiersLayout->addWidget(m_hotkeyModifierAlt);
  modifiersLayout->addWidget(m_hotkeyModifierShift);
  modifiersLayout->addStretch();
  mainHotkeyLayout->addRow("Modifiers:", modifiersLayout);

  layout->addWidget(mainHotkeyGroup);

  // === Alternative Toggle Hotkey Group ===
  QGroupBox *altHotkeyGroup = new QGroupBox("Alternative Open Hotkey", tab);
  QFormLayout *altHotkeyLayout = new QFormLayout(altHotkeyGroup);

  altHotkeyLayout->addRow(
      new QLabel("Open window and execute Alternative Script", this));

  m_altToggleKeyLineEdit = new QLineEdit(this);
  m_altToggleKeyLineEdit->setToolTip(
      "Enter key code (e.g., 83 for 'S') or character");
  m_altToggleKeyLineEdit->setMaxLength(3);
  m_altToggleKeyLineEdit->setPlaceholderText("83 or S");
  altHotkeyLayout->addRow("Key:", m_altToggleKeyLineEdit);

  m_altToggleModifierCtrl = new QCheckBox("Ctrl", this);
  m_altToggleModifierAlt = new QCheckBox("Alt", this);
  m_altToggleModifierShift = new QCheckBox("Shift", this);

  QHBoxLayout *altModifiersLayout = new QHBoxLayout();
  altModifiersLayout->addWidget(m_altToggleModifierCtrl);
  altModifiersLayout->addWidget(m_altToggleModifierAlt);
  altModifiersLayout->addWidget(m_altToggleModifierShift);
  altModifiersLayout->addStretch();
  altHotkeyLayout->addRow("Modifiers:", altModifiersLayout);

  layout->addWidget(altHotkeyGroup);

  // === Window Settings Group ===
  QGroupBox *windowGroup = new QGroupBox("Window Settings", tab);
  QFormLayout *windowLayout = new QFormLayout(windowGroup);

  m_alwaysOnTopCheckBox = new QCheckBox(this);
  m_alwaysOnTopCheckBox->setToolTip("Keep window always on top");
  windowLayout->addRow("Always on Top:", m_alwaysOnTopCheckBox);

  m_opacitySpinBox = new QSpinBox(this);
  m_opacitySpinBox->setRange(20, 100);
  m_opacitySpinBox->setSuffix("%");
  m_opacitySpinBox->setToolTip("Window transparency (20-100%)");
  windowLayout->addRow("Opacity:", m_opacitySpinBox);

  connect(m_opacitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &SettingsDialog::onOpacityChanged);

  layout->addWidget(windowGroup);

  // === General Settings Group ===
  QGroupBox *generalGroup = new QGroupBox("General Settings", tab);
  QFormLayout *generalLayout = new QFormLayout(generalGroup);

  m_showWindowOnStartupCheckBox = new QCheckBox(this);
  m_showWindowOnStartupCheckBox->setToolTip(
      "Show window when application starts");
  generalLayout->addRow("Show on Startup:", m_showWindowOnStartupCheckBox);

  m_autoStartOnLoginCheckBox = new QCheckBox(this);
  m_autoStartOnLoginCheckBox->setToolTip(
      "Start application when Windows logs in");
  generalLayout->addRow("Auto-start on Login:", m_autoStartOnLoginCheckBox);

  m_minimizeToTrayCheckBox = new QCheckBox(this);
  m_minimizeToTrayCheckBox->setToolTip("Minimize to tray instead of closing");
  generalLayout->addRow("Minimize to Tray:", m_minimizeToTrayCheckBox);

  m_darkThemeCheckBox = new QCheckBox(this);
  m_darkThemeCheckBox->setToolTip("Enable dark theme");
  generalLayout->addRow("Dark Theme:", m_darkThemeCheckBox);

  // m_autoTranslateCheckBox removed as execution is now mandatory

  layout->addWidget(generalGroup);
  layout->addStretch();
}

void SettingsDialog::setupResourcesTab(QWidget *tab) {
  QVBoxLayout *layout = new QVBoxLayout(tab);

  // === Section I: Startup Behavior ===
  QGroupBox *startupGroup = new QGroupBox("Startup Behavior", tab);
  QVBoxLayout *startupLayout = new QVBoxLayout(startupGroup);

  m_startupLastUsedRadio = new QRadioButton("Open last used resource", this);
  m_startupSelectedRadio =
      new QRadioButton("Always open selected resource:", this);

  // Apply theme
  bool darkTheme = AppConfig::instance()->getDarkTheme();
  if (darkTheme) {
    QString radioStyle = "QRadioButton { color: #ddd; }";
    m_startupLastUsedRadio->setStyleSheet(radioStyle);
    m_startupSelectedRadio->setStyleSheet(radioStyle);
  }

  QButtonGroup *startupButtonGroup = new QButtonGroup(this);
  startupButtonGroup->addButton(m_startupLastUsedRadio);
  startupButtonGroup->addButton(m_startupSelectedRadio);

  m_defaultResourceCombo = new QComboBox(this);
  m_defaultResourceCombo->setEnabled(false);

  connect(m_startupSelectedRadio, &QRadioButton::toggled,
          m_defaultResourceCombo, &QComboBox::setEnabled);
  connect(m_startupLastUsedRadio, &QRadioButton::toggled, this,
          &SettingsDialog::onStartupModeChanged);
  connect(m_startupSelectedRadio, &QRadioButton::toggled, this,
          &SettingsDialog::onStartupModeChanged);

  startupLayout->addWidget(m_startupLastUsedRadio);

  QHBoxLayout *selectedLayout = new QHBoxLayout();
  selectedLayout->addWidget(m_startupSelectedRadio);
  selectedLayout->addWidget(m_defaultResourceCombo, 1);
  startupLayout->addLayout(selectedLayout);

  layout->addWidget(startupGroup);

  // === Section II: Resource List ===
  QGroupBox *resourceListGroup = new QGroupBox("Resources", tab);
  QVBoxLayout *resourceListLayout = new QVBoxLayout(resourceListGroup);

  m_resourceScrollArea = new QScrollArea(this);
  m_resourceScrollArea->setWidgetResizable(true);
  m_resourceScrollArea->setMinimumHeight(200);

  QWidget *scrollContent = new QWidget();
  scrollContent->setObjectName("scrollContent");
  scrollContent->setStyleSheet("background: transparent;");
  m_resourcePanelsLayout = new QVBoxLayout(scrollContent);
  m_resourcePanelsLayout->addStretch();
  m_resourceScrollArea->setWidget(scrollContent);

  resourceListLayout->addWidget(m_resourceScrollArea);

  m_addResourceButton = new QPushButton("+ Add Resource", this);
  connect(m_addResourceButton, &QPushButton::clicked, this,
          &SettingsDialog::onAddResourceClicked);
  resourceListLayout->addWidget(m_addResourceButton);

  layout->addWidget(resourceListGroup, 1);

  // === Section III: Import/Export ===
  QGroupBox *presetGroup = new QGroupBox("Presets", tab);
  QHBoxLayout *presetLayout = new QHBoxLayout(presetGroup);

  m_importPresetsButton = new QPushButton("Import Presets...", this);
  m_importPresetsButton->setToolTip(
      "Import resources from JSON file (appends to existing)");
  connect(m_importPresetsButton, &QPushButton::clicked, this,
          &SettingsDialog::onImportPresetsClicked);
  presetLayout->addWidget(m_importPresetsButton);

  m_exportPresetsButton = new QPushButton("Export Presets...", this);
  m_exportPresetsButton->setToolTip("Export all resources to JSON file");
  connect(m_exportPresetsButton, &QPushButton::clicked, this,
          &SettingsDialog::onExportPresetsClicked);
  presetLayout->addWidget(m_exportPresetsButton);

  presetLayout->addStretch();

  layout->addWidget(presetGroup);
}

// Helper class for collapsible resource panel
class ResourcePanel : public QWidget {
public:
  ResourcePanel(const WebResource &resource, bool darkTheme,
                QWidget *parent = nullptr)
      : QWidget(parent), m_resourceId(resource.id), m_isExpanded(false) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header button
    m_toggleButton = new QToolButton(this);
    if (darkTheme) {
      m_toggleButton->setStyleSheet(
          "QToolButton { border: none; background: #3b3b3b; color: #ddd; "
          "text-align: left; "
          "padding: 5px; font-weight: bold; } QToolButton:hover { background: "
          "#4e4e4e; }");
    } else {
      m_toggleButton->setStyleSheet(
          "QToolButton { border: none; background: #e0e0e0; color: #000; "
          "text-align: left; "
          "padding: 5px; font-weight: bold; } QToolButton:hover { background: "
          "#d0d0d0; }");
    }
    m_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggleButton->setArrowType(Qt::RightArrow);
    m_toggleButton->setText(resource.name);
    m_toggleButton->setCheckable(true);
    m_toggleButton->setChecked(false);
    m_toggleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(m_toggleButton, &QToolButton::toggled, this,
            &ResourcePanel::onToggled);
    mainLayout->addWidget(m_toggleButton);

    // Content area
    m_contentWidget = new QGroupBox(this); // Use GroupBox for framing
    m_contentWidget->setObjectName("contentWidget");
    m_contentWidget->setVisible(false); // Collapsed by default

    // Inner layout
    m_contentLayout = new QVBoxLayout(m_contentWidget);

    // Resource Name
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Name:"));
    QLineEdit *nameEdit = new QLineEdit(resource.name);
    nameEdit->setObjectName("nameEdit");
    connect(nameEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_toggleButton->setText(text); });
    nameLayout->addWidget(nameEdit, 1);
    m_contentLayout->addLayout(nameLayout);

    // URL
    QHBoxLayout *urlLayout = new QHBoxLayout();
    urlLayout->addWidget(new QLabel("URL:"));
    QLineEdit *urlEdit = new QLineEdit(resource.url);
    urlEdit->setObjectName("urlEdit");
    urlEdit->setPlaceholderText("https://example.com");
    urlLayout->addWidget(urlEdit, 1);
    m_contentLayout->addLayout(urlLayout);

    // Zoom Factor
    QHBoxLayout *zoomLayout = new QHBoxLayout();
    zoomLayout->addWidget(new QLabel("Zoom Level (%):"));
    QSpinBox *zoomSpinBox = new QSpinBox();
    zoomSpinBox->setObjectName("zoomSpinBox");
    zoomSpinBox->setRange(30, 300);
    zoomSpinBox->setSingleStep(10);
    zoomSpinBox->setValue(static_cast<int>(resource.zoomFactor * 100));
    zoomSpinBox->setSuffix("%");
    zoomLayout->addWidget(zoomSpinBox);
    zoomLayout->addStretch();
    m_contentLayout->addLayout(zoomLayout);

    // Init Script
    m_contentLayout->addWidget(
        new QLabel("Initialization Script (Executed once on load):"));

    QPlainTextEdit *initScriptEdit = new QPlainTextEdit();
    initScriptEdit->setObjectName("initScriptEdit");
    initScriptEdit->setPlainText(resource.initScript);
    initScriptEdit->setMaximumHeight(60);
    initScriptEdit->setPlaceholderText(
        "// JavaScript executed when page finishes loading");
    m_contentLayout->addWidget(initScriptEdit);

    // Open Script
    m_contentLayout->addWidget(new QLabel("Open Script (JavaScript):"));

    QPlainTextEdit *openScriptEdit = new QPlainTextEdit();
    openScriptEdit->setObjectName("openScriptEdit");
    openScriptEdit->setPlainText(resource.openScript);
    openScriptEdit->setMaximumHeight(60);
    openScriptEdit->setPlaceholderText("// JavaScript executed on normal open");
    m_contentLayout->addWidget(openScriptEdit);

    // Alt Open Script
    m_contentLayout->addWidget(
        new QLabel("Alternative Open Script (JavaScript):"));

    QPlainTextEdit *altScriptEdit = new QPlainTextEdit();
    altScriptEdit->setObjectName("altScriptEdit");
    altScriptEdit->setPlainText(resource.altOpenScript);
    altScriptEdit->setMaximumHeight(60);
    altScriptEdit->setPlaceholderText(
        "// JavaScript executed on alternative open");
    m_contentLayout->addWidget(altScriptEdit);

    mainLayout->addWidget(m_contentWidget);
  }

  QString getResourceId() const { return m_resourceId; }

  QGroupBox *getContentWidget() const { return m_contentWidget; }

  void setDeleteAction(std::function<void()> action) {
    QPushButton *deleteButton = new QPushButton("Delete Resource", this);
    deleteButton->setStyleSheet("color: red;");
    connect(deleteButton, &QPushButton::clicked, action);
    m_contentLayout->addWidget(deleteButton);
  }

  // Programmatically expand/collapse
  void setExpanded(bool expanded) {
    m_toggleButton->setChecked(expanded);
    onToggled(expanded);
  }

private slots:
  void onToggled(bool checked) {
    m_contentWidget->setVisible(checked);
    m_toggleButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
  }

private:
  QString m_resourceId;
  QToolButton *m_toggleButton;
  QGroupBox *m_contentWidget;
  QVBoxLayout *m_contentLayout;
  bool m_isExpanded;
};

// ... in SettingsDialog methods ...

QWidget *SettingsDialog::createResourcePanel(const WebResource &resource) {
  bool darkTheme = AppConfig::instance()->getDarkTheme();
  ResourcePanel *panel = new ResourcePanel(resource, darkTheme, this);
  panel->setDeleteAction(
      [this, resource]() { onResourceDeleteClicked(resource.id); });

  // Auto-expand if new (name is default)
  if (resource.name == "New Resource") {
    panel->setExpanded(true);
  }

  return panel;
}

void SettingsDialog::onResourceDeleteClicked(const QString &resourceId) {
  int result = QMessageBox::question(
      this, "Delete Resource", "Are you sure you want to delete this resource?",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (result == QMessageBox::Yes) {
    ResourceManager::instance()->removeResource(resourceId);
    refreshResourcePanels();
  }
}

void SettingsDialog::onAddResourceClicked() {
  WebResource newResource = WebResource::create("New Resource", "https://");
  newResource.order = ResourceManager::instance()->getResourceCount();
  ResourceManager::instance()->addResource(newResource);
  refreshResourcePanels();
}

void SettingsDialog::onImportPresetsClicked() {
  QString filePath = QFileDialog::getOpenFileName(
      this, "Import Presets", QString(), "JSON Files (*.json)");

  if (!filePath.isEmpty()) {
    if (ResourceManager::instance()->importPresets(filePath)) {
      refreshResourcePanels();
      QMessageBox::information(this, "Import Successful",
                               "Resources imported successfully.");
    } else {
      QMessageBox::warning(this, "Import Failed",
                           "Failed to import resources from file.");
    }
  }
}

void SettingsDialog::onExportPresetsClicked() {
  QString filePath = QFileDialog::getSaveFileName(
      this, "Export Presets", "tinytools_presets.json", "JSON Files (*.json)");

  if (!filePath.isEmpty()) {
    if (ResourceManager::instance()->exportPresets(filePath)) {
      QMessageBox::information(this, "Export Successful",
                               "Resources exported successfully.");
    } else {
      QMessageBox::warning(this, "Export Failed",
                           "Failed to export resources to file.");
    }
  }
}

void SettingsDialog::onStartupModeChanged() {
  // This will be saved when Apply/OK is clicked
}

void SettingsDialog::loadSettings() {
  qDebug() << "SettingsDialog::loadSettings() - ENTRY";

  AppConfig *config = AppConfig::instance();
  if (!config->load()) {
    qWarning() << "Failed to load settings";
    return;
  }

  // Load Main Toggle hotkey
  int mainKey = config->getHotkeyKey(HotkeyType::MainToggle);
  m_hotkeyKeyLineEdit->setText(QString::number(mainKey));

  Qt::KeyboardModifiers mainMod =
      config->getHotkeyModifiers(HotkeyType::MainToggle);
  m_hotkeyModifierCtrl->setChecked(mainMod & Qt::ControlModifier);
  m_hotkeyModifierAlt->setChecked(mainMod & Qt::AltModifier);
  m_hotkeyModifierShift->setChecked(mainMod & Qt::ShiftModifier);

  // Load Alternative Toggle hotkey
  int altKey = config->getHotkeyKey(HotkeyType::AlternativeToggle);
  m_altToggleKeyLineEdit->setText(QString::number(altKey));

  Qt::KeyboardModifiers altMod =
      config->getHotkeyModifiers(HotkeyType::AlternativeToggle);
  m_altToggleModifierCtrl->setChecked(altMod & Qt::ControlModifier);
  m_altToggleModifierAlt->setChecked(altMod & Qt::AltModifier);
  m_altToggleModifierShift->setChecked(altMod & Qt::ShiftModifier);

  // Load window settings
  m_alwaysOnTopCheckBox->setChecked(config->getAlwaysOnTop());
  m_opacitySpinBox->setValue(config->getWindowOpacity());

  // Load general settings
  m_showWindowOnStartupCheckBox->setChecked(config->getShowWindowOnStartup());
  m_autoStartOnLoginCheckBox->setChecked(config->getAutoStartOnLogin());
  m_minimizeToTrayCheckBox->setChecked(config->getMinimizeToTray());
  m_darkThemeCheckBox->setChecked(config->getDarkTheme());
  m_darkThemeCheckBox->setChecked(config->getDarkTheme());
  // m_autoTranslateCheckBox->setChecked(config->getAutoTranslate());

  // Load Resources tab
  ResourceManager::instance()->loadFromConfig();

  // Startup mode
  if (ResourceManager::instance()->getStartupMode() ==
      ResourceManager::LastUsed) {
    m_startupLastUsedRadio->setChecked(true);
  } else {
    m_startupSelectedRadio->setChecked(true);
  }

  refreshResourcePanels();

  qDebug() << "SettingsDialog::loadSettings() - EXIT";
}

// ... (ResourcePanel class definition assumed correct) ...

void SettingsDialog::refreshResourcePanels() {
  qDebug() << "SettingsDialog::refreshResourcePanels() - ENTRY";

  // Clear existing panels
  for (QWidget *panel : m_resourcePanels) {
    m_resourcePanelsLayout->removeWidget(panel);
    delete panel;
  }
  m_resourcePanels.clear();

  // Rebuild from ResourceManager
  ResourceManager *rm = ResourceManager::instance();
  QList<WebResource> resources = rm->getAllResources();
  qDebug() << "Refreshing panels for" << resources.size() << "resources";

  for (const WebResource &resource : resources) {
    QWidget *panel = createResourcePanel(resource);
    m_resourcePanels.append(panel);
    // Insert before the stretch
    m_resourcePanelsLayout->insertWidget(m_resourcePanelsLayout->count() - 1,
                                         panel);
  }

  // Update default resource combo
  m_defaultResourceCombo->clear();
  for (const WebResource &resource : resources) {
    m_defaultResourceCombo->addItem(resource.name, resource.id);
  }

  // Select current default
  QString defaultId = rm->getDefaultResourceId();
  int index = m_defaultResourceCombo->findData(defaultId);
  if (index >= 0) {
    m_defaultResourceCombo->setCurrentIndex(index);
  }
  qDebug() << "SettingsDialog::refreshResourcePanels() - EXIT";
}

void SettingsDialog::saveSettings() {
  qDebug() << "SettingsDialog::saveSettings() - ENTRY";

  AppConfig *config = AppConfig::instance();

  // Save Main Toggle hotkey
  QString mainKeyText = m_hotkeyKeyLineEdit->text().trimmed();
  int mainKey = stringToKeyCode(mainKeyText);
  if (mainKey == -1)
    mainKey = Qt::Key_T;

  Qt::KeyboardModifiers mainMod = Qt::NoModifier;
  if (m_hotkeyModifierCtrl->isChecked())
    mainMod |= Qt::ControlModifier;
  if (m_hotkeyModifierAlt->isChecked())
    mainMod |= Qt::AltModifier;
  if (m_hotkeyModifierShift->isChecked())
    mainMod |= Qt::ShiftModifier;

  config->setHotkey(HotkeyType::MainToggle, mainKey, mainMod);

  // Save Alternative Toggle hotkey
  QString altKeyText = m_altToggleKeyLineEdit->text().trimmed();
  int altKey = stringToKeyCode(altKeyText);
  if (altKey == -1)
    altKey = Qt::Key_S;

  Qt::KeyboardModifiers altMod = Qt::NoModifier;
  if (m_altToggleModifierCtrl->isChecked())
    altMod |= Qt::ControlModifier;
  if (m_altToggleModifierAlt->isChecked())
    altMod |= Qt::AltModifier;
  if (m_altToggleModifierShift->isChecked())
    altMod |= Qt::ShiftModifier;

  config->setHotkey(HotkeyType::AlternativeToggle, altKey, altMod);

  // Save window settings
  config->setAlwaysOnTop(m_alwaysOnTopCheckBox->isChecked());
  config->setWindowOpacity(m_opacitySpinBox->value());

  // Save general settings
  config->setShowWindowOnStartup(m_showWindowOnStartupCheckBox->isChecked());
  config->setAutoStartOnLogin(m_autoStartOnLoginCheckBox->isChecked());
  config->setMinimizeToTray(m_minimizeToTrayCheckBox->isChecked());
  config->setDarkTheme(m_darkThemeCheckBox->isChecked());
  config->setDarkTheme(m_darkThemeCheckBox->isChecked());
  // config->setAutoTranslate(m_autoTranslateCheckBox->isChecked());

  // Save resource panels
  qDebug() << "Saving" << m_resourcePanels.size() << "resource panels...";
  for (QWidget *widget : m_resourcePanels) {
    ResourcePanel *panel = dynamic_cast<ResourcePanel *>(widget);
    if (panel) {
      QString resourceId = panel->getResourceId();
      qDebug() << "Saving panel for resource:" << resourceId;
      saveResourceFromPanel(panel, resourceId);
    } else {
      qWarning()
          << "Found widget in m_resourcePanels that is not a ResourcePanel!";
    }
  }

  // Save startup mode
  ResourceManager *rm = ResourceManager::instance();
  if (m_startupLastUsedRadio->isChecked()) {
    rm->setStartupMode(ResourceManager::LastUsed);
  } else {
    rm->setStartupMode(ResourceManager::SelectedResource);
    QString selectedId = m_defaultResourceCombo->currentData().toString();
    rm->setDefaultResourceId(selectedId);
  }

  // Save to disk
  qDebug() << "Calling ResourceManager::saveToConfig()...";
  if (rm->saveToConfig()) {
    qDebug() << "ResourceManager::saveToConfig() SUCCESS";
  } else {
    qCritical() << "ResourceManager::saveToConfig() FAILED";
  }

  // AppConfig::save is already called by rm->saveToConfig, but let's be safe
  // and check if config is dirty? Actually, saveToConfig updates the config
  // object and calls save(), so calling it again is redundant but harmless.
  // Let's rely on rm->saveToConfig() for the actual disk write of resources.

  qDebug() << "SettingsDialog::saveSettings() - EXIT";
}

void SettingsDialog::saveResourceFromPanel(QWidget *widget,
                                           const QString &resourceId) {
  ResourcePanel *panel = dynamic_cast<ResourcePanel *>(widget);
  if (!panel)
    return;

  WebResource resource =
      ResourceManager::instance()->getResourceById(resourceId);
  if (!resource.isValid()) {
    qWarning() << "Invalid resource found during save:" << resourceId;
    return;
  }

  QGroupBox *content = panel->getContentWidget();

  QLineEdit *nameEdit = content->findChild<QLineEdit *>("nameEdit");
  QLineEdit *urlEdit = content->findChild<QLineEdit *>("urlEdit");
  QSpinBox *zoomSpinBox = content->findChild<QSpinBox *>("zoomSpinBox");
  QPlainTextEdit *initScriptEdit =
      content->findChild<QPlainTextEdit *>("initScriptEdit");
  QPlainTextEdit *openScriptEdit =
      content->findChild<QPlainTextEdit *>("openScriptEdit");
  QPlainTextEdit *altScriptEdit =
      content->findChild<QPlainTextEdit *>("altScriptEdit");

  if (nameEdit)
    resource.name = nameEdit->text();
  if (urlEdit)
    resource.url = urlEdit->text();
  if (zoomSpinBox)
    resource.zoomFactor = zoomSpinBox->value() / 100.0;
  if (initScriptEdit)
    resource.initScript = initScriptEdit->toPlainText();
  if (openScriptEdit)
    resource.openScript = openScriptEdit->toPlainText();
  if (altScriptEdit)
    resource.altOpenScript = altScriptEdit->toPlainText();

  ResourceManager::instance()->updateResource(resource);
}

void SettingsDialog::applySettings() { saveSettings(); }

void SettingsDialog::onAccepted() {
  saveSettings();
  accept();
}

void SettingsDialog::onResetClicked() {
  // Reset General tab
  m_hotkeyKeyLineEdit->setText("84");
  m_hotkeyModifierCtrl->setChecked(true);
  m_hotkeyModifierAlt->setChecked(true);
  m_hotkeyModifierShift->setChecked(false);

  m_altToggleKeyLineEdit->setText("83");
  m_altToggleModifierCtrl->setChecked(true);
  m_altToggleModifierAlt->setChecked(true);
  m_altToggleModifierShift->setChecked(false);

  m_alwaysOnTopCheckBox->setChecked(true);
  m_opacitySpinBox->setValue(90);

  m_showWindowOnStartupCheckBox->setChecked(true);
  m_autoStartOnLoginCheckBox->setChecked(false);
  m_minimizeToTrayCheckBox->setChecked(true);
  m_darkThemeCheckBox->setChecked(false);
  m_darkThemeCheckBox->setChecked(false);
  // m_autoTranslateCheckBox->setChecked(false);

  // Reset Resources tab
  m_startupLastUsedRadio->setChecked(true);

  applySettings();
}

void SettingsDialog::onOpacityChanged(int value) { Q_UNUSED(value); }

int SettingsDialog::stringToKeyCode(const QString &text) {
  if (text.isEmpty())
    return -1;

  bool ok;
  int keyCode = text.toInt(&ok);
  if (ok && keyCode >= 0 && keyCode <= 255) {
    return keyCode;
  }

  if (text.length() == 1) {
    QChar ch = text.at(0).toUpper();
    if (ch >= 'A' && ch <= 'Z')
      return ch.unicode();
    if (ch >= '0' && ch <= '9')
      return ch.unicode();
  }

  return -1;
}
