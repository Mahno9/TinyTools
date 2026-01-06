#pragma once
#include <QDialog>
#include <QPointer>

class QSpinBox;
class QCheckBox;
class QLineEdit;
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
    int stringToKeyCode(const QString& text);
    
    QLineEdit* m_hotkeyKeyLineEdit;
    QCheckBox* m_hotkeyModifierCtrl;
    QCheckBox* m_hotkeyModifierAlt;
    QCheckBox* m_hotkeyModifierShift;
    QCheckBox* m_hotkeyModifierMeta;
    QCheckBox* m_alwaysOnTopCheckBox;
    QSpinBox* m_opacitySpinBox;
    QCheckBox* m_autoStartCheckBox;
    QCheckBox* m_minimizeToTrayCheckBox;
    QCheckBox* m_darkThemeCheckBox;
    QPushButton* m_resetButton;
    QPushButton* m_applyButton;
    
    // Translation settings
    QCheckBox* m_autoTranslateCheckBox;
};
