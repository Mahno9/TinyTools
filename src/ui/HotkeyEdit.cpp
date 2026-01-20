#include "HotkeyEdit.h"
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>


HotkeyEdit::HotkeyEdit(QWidget *parent) : QWidget(parent) {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(4);

  m_lineEdit = new QLineEdit(this);
  m_lineEdit->setPlaceholderText("Ctrl+Alt+T");
  m_lineEdit->setToolTip("Enter hotkey (e.g., Ctrl+Alt+T) or click Record");

  m_recordButton = new QPushButton("Record", this);
  m_recordButton->setToolTip("Click and press the desired key combination");
  m_recordButton->setFixedWidth(70);

  layout->addWidget(m_lineEdit, 1);
  layout->addWidget(m_recordButton);

  connect(m_recordButton, &QPushButton::clicked, this,
          &HotkeyEdit::onRecordClicked);
  connect(m_lineEdit, &QLineEdit::textEdited, this, &HotkeyEdit::onTextEdited);

  // Install event filter on line edit to capture key presses during recording
  m_lineEdit->installEventFilter(this);
}

void HotkeyEdit::setHotkey(int key, Qt::KeyboardModifiers modifiers) {
  m_key = key;
  m_modifiers = modifiers;
  updateDisplay();
}

QString HotkeyEdit::hotkeyString() const {
  if (m_key == 0)
    return QString();

  QStringList parts;
  if (m_modifiers & Qt::ControlModifier)
    parts << "Ctrl";
  if (m_modifiers & Qt::AltModifier)
    parts << "Alt";
  if (m_modifiers & Qt::ShiftModifier)
    parts << "Shift";
  if (m_modifiers & Qt::MetaModifier)
    parts << "Win";

  parts << keyToString(m_key);
  return parts.join("+");
}

void HotkeyEdit::setHotkeyString(const QString &str) {
  parseHotkeyString(str);
  updateDisplay();
}

void HotkeyEdit::startRecording() {
  if (m_recording)
    return;

  m_recording = true;
  m_recordButton->setText("...");
  m_lineEdit->setText("Press hotkey...");
  m_lineEdit->setFocus();
  m_lineEdit->setReadOnly(true);

  // Grab keyboard to ensure we get all key events
  m_lineEdit->grabKeyboard();

  emit recordingStarted();
  qDebug() << "HotkeyEdit: Recording started";
}

void HotkeyEdit::stopRecording() {
  if (!m_recording)
    return;

  m_recording = false;
  m_recordButton->setText("Record");
  m_lineEdit->setReadOnly(false);
  m_lineEdit->releaseKeyboard();

  updateDisplay();

  emit recordingStopped();
  qDebug() << "HotkeyEdit: Recording stopped";
}

void HotkeyEdit::onRecordClicked() {
  if (m_recording) {
    stopRecording();
  } else {
    startRecording();
  }
}

void HotkeyEdit::onTextEdited(const QString &text) {
  // Parse manually entered hotkey
  if (!m_recording) {
    int oldKey = m_key;
    Qt::KeyboardModifiers oldMods = m_modifiers;

    parseHotkeyString(text);

    if (oldKey != m_key || oldMods != m_modifiers) {
      emit hotkeyChanged(m_key, m_modifiers);
    }
  }
}

bool HotkeyEdit::eventFilter(QObject *watched, QEvent *event) {
  if (watched == m_lineEdit && m_recording) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

      int key = keyEvent->key();
      Qt::KeyboardModifiers mods = keyEvent->modifiers();

      // Ignore pure modifier keys
      if (key == Qt::Key_Control || key == Qt::Key_Shift ||
          key == Qt::Key_Alt || key == Qt::Key_Meta || key == Qt::Key_AltGr) {
        return true;
      }

      // Escape cancels recording
      if (key == Qt::Key_Escape) {
        stopRecording();
        return true;
      }

      // Remove numpad modifier if present
      mods &= ~Qt::KeypadModifier;

      // Save the new hotkey
      m_key = key;
      m_modifiers = mods;

      qDebug() << "HotkeyEdit: Recorded" << hotkeyString();

      stopRecording();
      emit hotkeyChanged(m_key, m_modifiers);

      return true;
    }

    // Block all other key events during recording
    if (event->type() == QEvent::KeyRelease) {
      return true;
    }
  }

  return QWidget::eventFilter(watched, event);
}

void HotkeyEdit::focusOutEvent(QFocusEvent *event) {
  if (m_recording) {
    stopRecording();
  }
  QWidget::focusOutEvent(event);
}

void HotkeyEdit::updateDisplay() {
  m_lineEdit->blockSignals(true);
  m_lineEdit->setText(hotkeyString());
  m_lineEdit->blockSignals(false);
}

void HotkeyEdit::parseHotkeyString(const QString &str) {
  m_key = 0;
  m_modifiers = Qt::NoModifier;

  if (str.isEmpty())
    return;

  QStringList parts = str.split('+', Qt::SkipEmptyParts);

  for (const QString &part : parts) {
    QString p = part.trimmed().toLower();

    if (p == "ctrl" || p == "control") {
      m_modifiers |= Qt::ControlModifier;
    } else if (p == "alt") {
      m_modifiers |= Qt::AltModifier;
    } else if (p == "shift") {
      m_modifiers |= Qt::ShiftModifier;
    } else if (p == "win" || p == "meta" || p == "super") {
      m_modifiers |= Qt::MetaModifier;
    } else {
      // It's the key
      m_key = stringToKey(part.trimmed());
    }
  }
}

QString HotkeyEdit::keyToString(int key) const {
  // Handle special keys
  switch (key) {
  case Qt::Key_Space:
    return "Space";
  case Qt::Key_Return:
    return "Enter";
  case Qt::Key_Enter:
    return "Enter";
  case Qt::Key_Tab:
    return "Tab";
  case Qt::Key_Backspace:
    return "Backspace";
  case Qt::Key_Delete:
    return "Delete";
  case Qt::Key_Insert:
    return "Insert";
  case Qt::Key_Home:
    return "Home";
  case Qt::Key_End:
    return "End";
  case Qt::Key_PageUp:
    return "PageUp";
  case Qt::Key_PageDown:
    return "PageDown";
  case Qt::Key_Left:
    return "Left";
  case Qt::Key_Right:
    return "Right";
  case Qt::Key_Up:
    return "Up";
  case Qt::Key_Down:
    return "Down";
  case Qt::Key_Escape:
    return "Esc";
  case Qt::Key_F1:
    return "F1";
  case Qt::Key_F2:
    return "F2";
  case Qt::Key_F3:
    return "F3";
  case Qt::Key_F4:
    return "F4";
  case Qt::Key_F5:
    return "F5";
  case Qt::Key_F6:
    return "F6";
  case Qt::Key_F7:
    return "F7";
  case Qt::Key_F8:
    return "F8";
  case Qt::Key_F9:
    return "F9";
  case Qt::Key_F10:
    return "F10";
  case Qt::Key_F11:
    return "F11";
  case Qt::Key_F12:
    return "F12";
  default:
    // For printable characters
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
      return QString(QChar(key));
    }
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
      return QString(QChar(key));
    }
    // Return key code for unknown keys
    return QString::number(key);
  }
}

int HotkeyEdit::stringToKey(const QString &str) const {
  QString s = str.toUpper();

  // Single character
  if (s.length() == 1) {
    QChar c = s[0];
    if (c.isLetter()) {
      return Qt::Key_A + (c.unicode() - 'A');
    }
    if (c.isDigit()) {
      return Qt::Key_0 + (c.unicode() - '0');
    }
  }

  // Check for numeric key code
  bool ok;
  int keyCode = str.toInt(&ok);
  if (ok && keyCode > 0) {
    return keyCode;
  }

  // Named keys
  if (s == "SPACE")
    return Qt::Key_Space;
  if (s == "ENTER" || s == "RETURN")
    return Qt::Key_Return;
  if (s == "TAB")
    return Qt::Key_Tab;
  if (s == "BACKSPACE")
    return Qt::Key_Backspace;
  if (s == "DELETE" || s == "DEL")
    return Qt::Key_Delete;
  if (s == "INSERT" || s == "INS")
    return Qt::Key_Insert;
  if (s == "HOME")
    return Qt::Key_Home;
  if (s == "END")
    return Qt::Key_End;
  if (s == "PAGEUP" || s == "PGUP")
    return Qt::Key_PageUp;
  if (s == "PAGEDOWN" || s == "PGDN")
    return Qt::Key_PageDown;
  if (s == "LEFT")
    return Qt::Key_Left;
  if (s == "RIGHT")
    return Qt::Key_Right;
  if (s == "UP")
    return Qt::Key_Up;
  if (s == "DOWN")
    return Qt::Key_Down;
  if (s == "ESC" || s == "ESCAPE")
    return Qt::Key_Escape;
  if (s == "F1")
    return Qt::Key_F1;
  if (s == "F2")
    return Qt::Key_F2;
  if (s == "F3")
    return Qt::Key_F3;
  if (s == "F4")
    return Qt::Key_F4;
  if (s == "F5")
    return Qt::Key_F5;
  if (s == "F6")
    return Qt::Key_F6;
  if (s == "F7")
    return Qt::Key_F7;
  if (s == "F8")
    return Qt::Key_F8;
  if (s == "F9")
    return Qt::Key_F9;
  if (s == "F10")
    return Qt::Key_F10;
  if (s == "F11")
    return Qt::Key_F11;
  if (s == "F12")
    return Qt::Key_F12;

  return 0;
}
