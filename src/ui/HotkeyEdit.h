#pragma once
#include <QKeySequence>
#include <QWidget>


class QLineEdit;
class QPushButton;

/**
 * @brief HotkeyEdit - Custom widget for recording and editing hotkey
 * combinations
 *
 * Provides two ways to set hotkey:
 * 1. Click "Record" and press the key combination
 * 2. Type the combination manually (e.g., "Ctrl+Alt+T")
 */
class HotkeyEdit : public QWidget {
  Q_OBJECT

public:
  explicit HotkeyEdit(QWidget *parent = nullptr);

  // Get/set hotkey as key code and modifiers
  int key() const { return m_key; }
  Qt::KeyboardModifiers modifiers() const { return m_modifiers; }

  void setHotkey(int key, Qt::KeyboardModifiers modifiers);

  // Get/set as string (e.g., "Ctrl+Alt+T")
  QString hotkeyString() const;
  void setHotkeyString(const QString &str);

  // Check if currently recording
  bool isRecording() const { return m_recording; }

signals:
  void hotkeyChanged(int key, Qt::KeyboardModifiers modifiers);
  void recordingStarted();
  void recordingStopped();

public slots:
  void startRecording();
  void stopRecording();

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;

private slots:
  void onRecordClicked();
  void onTextEdited(const QString &text);

private:
  void updateDisplay();
  void parseHotkeyString(const QString &str);
  QString keyToString(int key) const;
  int stringToKey(const QString &str) const;

  QLineEdit *m_lineEdit;
  QPushButton *m_recordButton;

  int m_key = 0;
  Qt::KeyboardModifiers m_modifiers = Qt::NoModifier;
  bool m_recording = false;
};
