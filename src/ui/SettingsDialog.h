#pragma once
#include <QDialog>
#include <QPointer>
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
class QPlainTextEdit;
class QGroupBox;

struct WebResource;

/**
 * @brief SettingsDialog provides a two-tab settings interface
 * 
 * Tab 1 (General): Window, hotkey, and general settings
 * Tab 2 (Resources): Resource management with collapsible panels
 */
class SettingsDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();
    
private slots:
    void onAccepted();
    void onResetClicked();
    void onOpacityChanged(int value);
    
    // Resources tab slots
    void onAddResourceClicked();
    void onImportPresetsClicked();
    void onExportPresetsClicked();
    void onStartupModeChanged();
    
private:
    void setupUI();
    void setupGeneralTab(QWidget* tab);
    void setupResourcesTab(QWidget* tab);
    void loadSettings();
    void saveSettings();
    void applySettings();
    int stringToKeyCode(const QString& text);
    
    // Resource panel management
    void refreshResourcePanels();
    QWidget* createResourcePanel(const WebResource& resource);
    void onResourceDeleteClicked(const QString& resourceId);
    void saveResourceFromPanel(QWidget* panel, const QString& resourceId);
    
    // Main tab widget
    QTabWidget* m_tabWidget;
    
    // === General Tab widgets ===
    // Hotkey Settings
    QLineEdit* m_hotkeyKeyLineEdit;
    QCheckBox* m_hotkeyModifierCtrl;
    QCheckBox* m_hotkeyModifierAlt;
    QCheckBox* m_hotkeyModifierShift;
    
    // Alternative Toggle hotkey
    QLineEdit* m_altToggleKeyLineEdit;
    QCheckBox* m_altToggleModifierCtrl;
    QCheckBox* m_altToggleModifierAlt;
    QCheckBox* m_altToggleModifierShift;
    
    // Window settings
    QCheckBox* m_alwaysOnTopCheckBox;
    QSpinBox* m_opacitySpinBox;
    
    // General settings
    QCheckBox* m_showWindowOnStartupCheckBox;
    QCheckBox* m_autoStartOnLoginCheckBox;
    QCheckBox* m_minimizeToTrayCheckBox;
    QCheckBox* m_darkThemeCheckBox;
    QCheckBox* m_autoTranslateCheckBox;
    
    // === Resources Tab widgets ===
    // Startup behavior
    QRadioButton* m_startupLastUsedRadio;
    QRadioButton* m_startupSelectedRadio;
    QComboBox* m_defaultResourceCombo;
    
    // Resource panels container
    QScrollArea* m_resourceScrollArea;
    QVBoxLayout* m_resourcePanelsLayout;
    QList<QWidget*> m_resourcePanels;
    
    // Import/Export buttons
    QPushButton* m_importPresetsButton;
    QPushButton* m_exportPresetsButton;
    QPushButton* m_addResourceButton;
    
    // Common buttons
    QPushButton* m_resetButton;
    QPushButton* m_applyButton;
};
