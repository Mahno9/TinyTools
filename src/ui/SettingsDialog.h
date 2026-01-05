#pragma once
#include <QDialog>
#include <QPointer>

class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;

class SettingsDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();
    
private slots:
    void onAccepted();
    void onResetClicked();
    void onOpacityChanged(int value);
    
private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void applySettings();
    
    QSpinBox* m_hotkeyKeySpinBox;
    QComboBox* m_hotkeyModifierCtrl;
    QComboBox* m_hotkeyModifierAlt;
    QComboBox* m_hotkeyModifierShift;
    QComboBox* m_hotkeyModifierMeta;
    QCheckBox* m_alwaysOnTopCheckBox;
    QSpinBox* m_opacitySpinBox;
    QCheckBox* m_autoStartCheckBox;
    QCheckBox* m_minimizeToTrayCheckBox;
    QComboBox* m_languageComboBox;
    QPushButton* m_resetButton;
    QPushButton* m_applyButton;
};
