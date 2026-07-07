#pragma once
#include "../models/WebResource.h"
#include <QDialog>
#include <QList>

class QSpinBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QComboBox;
class QRadioButton;
class QScrollArea;
class QVBoxLayout;
class HotkeyEdit;

/**
 * @brief SettingsDialog provides a two-tab settings interface
 *
 * Tab 1 (General): Window, hotkey, and general settings
 * Tab 2 (Resources): Resource management with collapsible panels
 *
 * Resource edits (including add/delete/import) are transactional: they are
 * held in a working copy and applied to ResourceManager only on OK/Apply.
 */
class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = nullptr);

signals:
  void testOpacity(int value);

private slots:
  void onAccepted();
  void onApplyClicked();
  void onResetClicked();
  void onOpacityChanged(int value);

  // Resources tab slots
  void onAddResourceClicked();
  void onImportPresetsClicked();
  void onExportPresetsClicked();
  void onResourceDeleteClicked(const QString &resourceId);

private:
  void setupUI();
  void setupGeneralTab(QWidget *tab);
  void setupResourcesTab(QWidget *tab);
  void loadSettings();
  bool saveSettings(); // false if validation failed (dialog stays open)
  void syncPanelsToWorking();
  bool validateSettings(QString *error) const;

  // Resource panel management
  void refreshResourcePanels();
  QWidget *createResourcePanel(const WebResource &resource);

  // Working copy of resources; applied to ResourceManager on save
  QList<WebResource> m_workingResources;

  // Main tab widget
  QTabWidget *m_tabWidget;

  // === General Tab widgets ===
  HotkeyEdit *m_mainHotkeyEdit;
  HotkeyEdit *m_altHotkeyEdit;

  QCheckBox *m_alwaysOnTopCheckBox;
  QSpinBox *m_opacitySpinBox;

  QCheckBox *m_showWindowOnStartupCheckBox;
  QCheckBox *m_autoStartOnLoginCheckBox;
  QCheckBox *m_minimizeToTrayCheckBox;
  QCheckBox *m_darkThemeCheckBox;

  // === Resources Tab widgets ===
  QRadioButton *m_startupLastUsedRadio;
  QRadioButton *m_startupSelectedRadio;
  QComboBox *m_defaultResourceCombo;

  QScrollArea *m_resourceScrollArea;
  QVBoxLayout *m_resourcePanelsLayout;
  QList<QWidget *> m_resourcePanels;

  QPushButton *m_importPresetsButton;
  QPushButton *m_exportPresetsButton;
  QPushButton *m_addResourceButton;

  QPushButton *m_resetButton;
  QPushButton *m_applyButton;
};
