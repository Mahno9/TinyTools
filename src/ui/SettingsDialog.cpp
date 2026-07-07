#include "SettingsDialog.h"
#include "../app/Constants.h"
#include "../core/HotkeyManager.h"
#include "../models/AppConfig.h"
#include "../models/ResourceManager.h"
#include "HotkeyEdit.h"

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
#include <QSet>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
bool hasRequiredModifier(Qt::KeyboardModifiers mods) {
  // Shift alone is not enough: a global "Shift+T" would swallow typing.
  return mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
}
} // namespace

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("TinyTools Settings");
  setMinimumSize(600, 600);
  setupUI();
  loadSettings();
}

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
        "QRadioButton { color: #ddd; }"
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

  QWidget *generalTab = new QWidget();
  setupGeneralTab(generalTab);
  m_tabWidget->addTab(generalTab, "General");

  QWidget *resourcesTab = new QWidget();
  setupResourcesTab(resourcesTab);
  m_tabWidget->addTab(resourcesTab, "Resources");

  mainLayout->addWidget(m_tabWidget);

  // Button Box (common for both tabs)
  QHBoxLayout *buttonLayout = new QHBoxLayout();

  m_resetButton = new QPushButton("Reset to Defaults", this);
  m_resetButton->setToolTip(
      "Reset the controls to default values (applied on OK/Apply)");
  buttonLayout->addWidget(m_resetButton);

  buttonLayout->addStretch();

  m_applyButton = new QPushButton("Apply", this);
  buttonLayout->addWidget(m_applyButton);

  QPushButton *okButton = new QPushButton("OK", this);
  buttonLayout->addWidget(okButton);

  QPushButton *cancelButton = new QPushButton("Cancel", this);
  buttonLayout->addWidget(cancelButton);

  mainLayout->addLayout(buttonLayout);

  connect(m_resetButton, &QPushButton::clicked, this,
          &SettingsDialog::onResetClicked);
  connect(m_applyButton, &QPushButton::clicked, this,
          &SettingsDialog::onApplyClicked);
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

  m_mainHotkeyEdit = new HotkeyEdit(this);
  m_mainHotkeyEdit->setToolTip(
      "Click Record and press the desired key combination (must include "
      "Ctrl, Alt or Win)");
  mainHotkeyLayout->addRow("Hotkey:", m_mainHotkeyEdit);

  layout->addWidget(mainHotkeyGroup);

  // === Alternative Toggle Hotkey Group ===
  QGroupBox *altHotkeyGroup = new QGroupBox("Alternative Open Hotkey", tab);
  QFormLayout *altHotkeyLayout = new QFormLayout(altHotkeyGroup);

  altHotkeyLayout->addRow(
      new QLabel("Open window and execute Alternative Script", this));

  m_altHotkeyEdit = new HotkeyEdit(this);
  m_altHotkeyEdit->setToolTip(
      "Click Record and press the desired key combination (must include "
      "Ctrl, Alt or Win)");
  altHotkeyLayout->addRow("Hotkey:", m_altHotkeyEdit);

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
  m_minimizeToTrayCheckBox->setToolTip(
      "Close button hides the window to tray instead of quitting");
  generalLayout->addRow("Minimize to Tray:", m_minimizeToTrayCheckBox);

  m_darkThemeCheckBox = new QCheckBox(this);
  m_darkThemeCheckBox->setToolTip("Enable dark theme");
  generalLayout->addRow("Dark Theme:", m_darkThemeCheckBox);

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

  QButtonGroup *startupButtonGroup = new QButtonGroup(this);
  startupButtonGroup->addButton(m_startupLastUsedRadio);
  startupButtonGroup->addButton(m_startupSelectedRadio);

  m_defaultResourceCombo = new QComboBox(this);
  m_defaultResourceCombo->setEnabled(false);

  connect(m_startupSelectedRadio, &QRadioButton::toggled,
          m_defaultResourceCombo, &QComboBox::setEnabled);

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
      : QWidget(parent), m_resourceId(resource.id) {
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

    m_contentLayout = new QVBoxLayout(m_contentWidget);

    // Resource Name + Enabled
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Name:"));
    QLineEdit *nameEdit = new QLineEdit(resource.name);
    nameEdit->setObjectName("nameEdit");
    connect(nameEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_toggleButton->setText(text); });
    nameLayout->addWidget(nameEdit, 1);

    QCheckBox *enabledCheck = new QCheckBox("Enabled");
    enabledCheck->setObjectName("enabledCheck");
    enabledCheck->setChecked(resource.isEnabled);
    enabledCheck->setToolTip("Disabled resources are hidden from the tab bar");
    nameLayout->addWidget(enabledCheck);
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

  // Reads current panel fields into the resource (id/order untouched).
  void applyTo(WebResource &resource) const {
    auto *nameEdit = m_contentWidget->findChild<QLineEdit *>("nameEdit");
    auto *urlEdit = m_contentWidget->findChild<QLineEdit *>("urlEdit");
    auto *zoomSpinBox = m_contentWidget->findChild<QSpinBox *>("zoomSpinBox");
    auto *enabledCheck = m_contentWidget->findChild<QCheckBox *>("enabledCheck");
    auto *initScriptEdit =
        m_contentWidget->findChild<QPlainTextEdit *>("initScriptEdit");
    auto *openScriptEdit =
        m_contentWidget->findChild<QPlainTextEdit *>("openScriptEdit");
    auto *altScriptEdit =
        m_contentWidget->findChild<QPlainTextEdit *>("altScriptEdit");

    if (nameEdit)
      resource.name = nameEdit->text().trimmed();
    if (urlEdit)
      resource.url = urlEdit->text().trimmed();
    if (zoomSpinBox)
      resource.zoomFactor = zoomSpinBox->value() / 100.0;
    if (enabledCheck)
      resource.isEnabled = enabledCheck->isChecked();
    if (initScriptEdit)
      resource.initScript = initScriptEdit->toPlainText();
    if (openScriptEdit)
      resource.openScript = openScriptEdit->toPlainText();
    if (altScriptEdit)
      resource.altOpenScript = altScriptEdit->toPlainText();
  }

  void setDeleteAction(std::function<void()> action) {
    QPushButton *deleteButton = new QPushButton("Delete Resource", this);
    deleteButton->setStyleSheet("color: red;");
    connect(deleteButton, &QPushButton::clicked, action);
    m_contentLayout->addWidget(deleteButton);
  }

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
};

QWidget *SettingsDialog::createResourcePanel(const WebResource &resource) {
  bool darkTheme = AppConfig::instance()->getDarkTheme();
  ResourcePanel *panel = new ResourcePanel(resource, darkTheme, this);
  panel->setDeleteAction(
      [this, id = resource.id]() { onResourceDeleteClicked(id); });

  // Auto-expand freshly added resources so the user can fill in the URL
  if (resource.url.isEmpty()) {
    panel->setExpanded(true);
  }

  return panel;
}

void SettingsDialog::onResourceDeleteClicked(const QString &resourceId) {
  int result = QMessageBox::question(
      this, "Delete Resource",
      "Delete this resource? The change is applied when you press OK or "
      "Apply.",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (result != QMessageBox::Yes) {
    return;
  }

  syncPanelsToWorking(); // keep edits made to other panels
  for (int i = 0; i < m_workingResources.size(); ++i) {
    if (m_workingResources[i].id == resourceId) {
      m_workingResources.removeAt(i);
      break;
    }
  }
  refreshResourcePanels();
}

void SettingsDialog::onAddResourceClicked() {
  if (m_workingResources.size() >= Constants::MAX_RESOURCES) {
    QMessageBox::information(
        this, "Limit reached",
        QString("A maximum of %1 resources is supported.")
            .arg(Constants::MAX_RESOURCES));
    return;
  }

  syncPanelsToWorking();
  WebResource newResource = WebResource::create("New Resource", QString());
  newResource.order = m_workingResources.size();
  m_workingResources.append(newResource);
  refreshResourcePanels();
}

void SettingsDialog::onImportPresetsClicked() {
  QString filePath = QFileDialog::getOpenFileName(
      this, "Import Presets", QString(), "JSON Files (*.json)");
  if (filePath.isEmpty()) {
    return;
  }

  // Imported presets contain JavaScript that runs inside the pages the user
  // opens - i.e. with access to their logged-in sessions. Make that explicit.
  int confirm = QMessageBox::warning(
      this, "Import Presets",
      "Imported resources can contain JavaScript that will run on the "
      "websites you open, with access to your logged-in sessions on those "
      "sites.\n\nOnly import presets from sources you trust.\n\nContinue?",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (confirm != QMessageBox::Yes) {
    return;
  }

  QString error;
  const QList<WebResource> parsed =
      ResourceManager::parsePresets(filePath, &error);
  if (!error.isEmpty()) {
    QMessageBox::warning(this, "Import Failed", error);
    return;
  }
  if (parsed.isEmpty()) {
    QMessageBox::warning(this, "Import Failed",
                         "The file contains no valid resources.");
    return;
  }

  syncPanelsToWorking();
  int imported = 0;
  for (const WebResource &resource : parsed) {
    if (m_workingResources.size() >= Constants::MAX_RESOURCES) {
      break;
    }
    m_workingResources.append(resource);
    imported++;
  }
  refreshResourcePanels();

  QString message = QString("Imported %1 resource(s).").arg(imported);
  if (imported < parsed.size()) {
    message += QString(" %1 skipped (limit of %2 reached).")
                   .arg(parsed.size() - imported)
                   .arg(Constants::MAX_RESOURCES);
  }
  message += "\nThe changes are applied when you press OK or Apply.";
  QMessageBox::information(this, "Import", message);
}

void SettingsDialog::onExportPresetsClicked() {
  QString filePath = QFileDialog::getSaveFileName(
      this, "Export Presets", "tinytools_presets.json", "JSON Files (*.json)");
  if (filePath.isEmpty()) {
    return;
  }

  syncPanelsToWorking();
  if (ResourceManager::writePresets(filePath, m_workingResources)) {
    QMessageBox::information(this, "Export Successful",
                             "Resources exported successfully.");
  } else {
    QMessageBox::warning(this, "Export Failed",
                         "Failed to export resources to file.");
  }
}

void SettingsDialog::loadSettings() {
  // Read the current in-memory state. Deliberately no disk reload here:
  // it would clobber unsaved in-memory changes (e.g. debounced zoom updates).
  AppConfig *config = AppConfig::instance();

  m_mainHotkeyEdit->setHotkey(config->getHotkeyKey(HotkeyType::MainToggle),
                              config->getHotkeyModifiers(HotkeyType::MainToggle));
  m_altHotkeyEdit->setHotkey(
      config->getHotkeyKey(HotkeyType::AlternativeToggle),
      config->getHotkeyModifiers(HotkeyType::AlternativeToggle));

  m_alwaysOnTopCheckBox->setChecked(config->getAlwaysOnTop());
  m_opacitySpinBox->setValue(config->getWindowOpacity());

  m_showWindowOnStartupCheckBox->setChecked(config->getShowWindowOnStartup());
  m_autoStartOnLoginCheckBox->setChecked(config->getAutoStartOnLogin());
  m_minimizeToTrayCheckBox->setChecked(config->getMinimizeToTray());
  m_darkThemeCheckBox->setChecked(config->getDarkTheme());

  ResourceManager *rm = ResourceManager::instance();
  m_workingResources = rm->getAllResources();

  if (rm->getStartupMode() == ResourceManager::LastUsed) {
    m_startupLastUsedRadio->setChecked(true);
  } else {
    m_startupSelectedRadio->setChecked(true);
  }

  refreshResourcePanels();
}

void SettingsDialog::refreshResourcePanels() {
  // Clear existing panels
  for (QWidget *panel : m_resourcePanels) {
    m_resourcePanelsLayout->removeWidget(panel);
    delete panel;
  }
  m_resourcePanels.clear();

  for (const WebResource &resource : m_workingResources) {
    QWidget *panel = createResourcePanel(resource);
    m_resourcePanels.append(panel);
    // Insert before the stretch
    m_resourcePanelsLayout->insertWidget(m_resourcePanelsLayout->count() - 1,
                                         panel);
  }

  // Update default resource combo
  QString previousDefault = m_defaultResourceCombo->currentData().toString();
  if (previousDefault.isEmpty()) {
    previousDefault = ResourceManager::instance()->getDefaultResourceId();
  }
  m_defaultResourceCombo->clear();
  for (const WebResource &resource : m_workingResources) {
    m_defaultResourceCombo->addItem(resource.name, resource.id);
  }
  int index = m_defaultResourceCombo->findData(previousDefault);
  if (index >= 0) {
    m_defaultResourceCombo->setCurrentIndex(index);
  }
}

void SettingsDialog::syncPanelsToWorking() {
  for (QWidget *widget : m_resourcePanels) {
    // All entries are created by createResourcePanel(), so the cast is safe.
    auto *panel = static_cast<ResourcePanel *>(widget);
    for (auto &resource : m_workingResources) {
      if (resource.id == panel->getResourceId()) {
        panel->applyTo(resource);
        break;
      }
    }
  }
}

bool SettingsDialog::validateSettings(QString *error) const {
  const int mainKey = m_mainHotkeyEdit->key();
  const Qt::KeyboardModifiers mainMod = m_mainHotkeyEdit->modifiers();
  const int altKey = m_altHotkeyEdit->key();
  const Qt::KeyboardModifiers altMod = m_altHotkeyEdit->modifiers();

  if (mainKey == 0) {
    *error = "Main hotkey is not set.";
    return false;
  }
  if (altKey == 0) {
    *error = "Alternative hotkey is not set.";
    return false;
  }
  if (!hasRequiredModifier(mainMod) || !hasRequiredModifier(altMod)) {
    *error = "Hotkeys must include Ctrl, Alt or Win - a bare key would "
             "intercept normal typing system-wide.";
    return false;
  }
  if (mainKey == altKey && mainMod == altMod) {
    *error = "Main and Alternative hotkeys must be different.";
    return false;
  }

  for (const WebResource &resource : m_workingResources) {
    if (resource.name.trimmed().isEmpty()) {
      *error = "A resource has an empty name.";
      return false;
    }
    if (!resource.isValid()) {
      *error = QString("Resource \"%1\" has an invalid URL (\"%2\"). "
                       "Only http/https URLs with a host are allowed.")
                   .arg(resource.name, resource.url);
      return false;
    }
  }
  return true;
}

bool SettingsDialog::saveSettings() {
  syncPanelsToWorking();

  QString error;
  if (!validateSettings(&error)) {
    QMessageBox::warning(this, "Invalid Settings", error);
    return false;
  }

  AppConfig *config = AppConfig::instance();

  config->setHotkey(HotkeyType::MainToggle, m_mainHotkeyEdit->key(),
                    m_mainHotkeyEdit->modifiers());
  config->setHotkey(HotkeyType::AlternativeToggle, m_altHotkeyEdit->key(),
                    m_altHotkeyEdit->modifiers());

  config->setAlwaysOnTop(m_alwaysOnTopCheckBox->isChecked());
  config->setWindowOpacity(m_opacitySpinBox->value());

  config->setShowWindowOnStartup(m_showWindowOnStartupCheckBox->isChecked());
  config->setAutoStartOnLogin(m_autoStartOnLoginCheckBox->isChecked());
  config->setMinimizeToTray(m_minimizeToTrayCheckBox->isChecked());
  config->setDarkTheme(m_darkThemeCheckBox->isChecked());

  // Apply the working copy to ResourceManager as a diff
  ResourceManager *rm = ResourceManager::instance();

  QSet<QString> workingIds;
  for (const auto &resource : m_workingResources) {
    workingIds.insert(resource.id);
  }
  const QList<WebResource> current = rm->getAllResources();
  for (const auto &resource : current) {
    if (!workingIds.contains(resource.id)) {
      rm->removeResource(resource.id);
    }
  }

  QStringList orderedIds;
  for (int i = 0; i < m_workingResources.size(); ++i) {
    WebResource resource = m_workingResources[i];
    resource.order = i;
    m_workingResources[i] = resource;
    if (rm->getResourceById(resource.id).isValid()) {
      rm->updateResource(resource);
    } else {
      rm->addResource(resource);
    }
    orderedIds << resource.id;
  }
  rm->reorderResources(orderedIds);

  // Startup mode
  if (m_startupLastUsedRadio->isChecked()) {
    rm->setStartupMode(ResourceManager::LastUsed);
  } else {
    rm->setStartupMode(ResourceManager::SelectedResource);
    rm->setDefaultResourceId(m_defaultResourceCombo->currentData().toString());
  }

  if (!rm->saveToConfig()) {
    QMessageBox::warning(this, "Save Failed",
                         "Failed to save settings to disk. Check the log for "
                         "details.");
    return false;
  }
  return true;
}

void SettingsDialog::onApplyClicked() {
  if (saveSettings()) {
    // Renames from the panels are now saved; refresh combo labels
    for (int i = 0; i < m_defaultResourceCombo->count(); ++i) {
      const QString id = m_defaultResourceCombo->itemData(i).toString();
      for (const auto &resource : m_workingResources) {
        if (resource.id == id) {
          m_defaultResourceCombo->setItemText(i, resource.name);
          break;
        }
      }
    }
  }
}

void SettingsDialog::onAccepted() {
  if (saveSettings()) {
    accept();
  }
}

void SettingsDialog::onResetClicked() {
  // Resets the controls only; nothing is applied until OK/Apply.
  m_mainHotkeyEdit->setHotkey(Qt::Key_T, Qt::ControlModifier | Qt::AltModifier);
  m_altHotkeyEdit->setHotkey(Qt::Key_S, Qt::ControlModifier | Qt::AltModifier);

  m_alwaysOnTopCheckBox->setChecked(true);
  m_opacitySpinBox->setValue(90);

  m_showWindowOnStartupCheckBox->setChecked(true);
  m_autoStartOnLoginCheckBox->setChecked(false);
  m_minimizeToTrayCheckBox->setChecked(true);
  m_darkThemeCheckBox->setChecked(false);

  m_startupLastUsedRadio->setChecked(true);
}

void SettingsDialog::onOpacityChanged(int value) { emit testOpacity(value); }
