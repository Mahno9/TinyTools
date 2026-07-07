#include "Logging.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QTextStream>

namespace {

QFile *s_file = nullptr;
QTextStream *s_stream = nullptr;
QMutex s_mutex;
bool s_debugEnabled = false;
QtMessageHandler s_previous = nullptr;
bool s_installed = false;

void messageHandler(QtMsgType type, const QMessageLogContext &,
                    const QString &msg) {
  if (type == QtDebugMsg && !s_debugEnabled)
    return;

  const char *level = "DEBUG";
  switch (type) {
  case QtDebugMsg:
    level = "DEBUG";
    break;
  case QtInfoMsg:
    level = "INFO";
    break;
  case QtWarningMsg:
    level = "WARN";
    break;
  case QtCriticalMsg:
    level = "CRITICAL";
    break;
  case QtFatalMsg:
    level = "FATAL";
    break;
  }

  QMutexLocker locker(&s_mutex);
  if (!s_stream)
    return;
  *s_stream << '['
            << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
            << "] [" << level << "] " << msg << '\n';
  s_stream->flush();
}

} // namespace

namespace Logging {

bool init(const QString &filePath, qint64 maxBytes) {
  shutdown();

  s_debugEnabled = qEnvironmentVariableIsSet("TINYTOOLS_DEBUG");

  QDir().mkpath(QFileInfo(filePath).absolutePath());

  // ponytail: single-generation size rotation; add timestamped archives if
  // anyone ever needs history.
  QFileInfo info(filePath);
  if (info.exists() && info.size() > maxBytes) {
    const QString old = filePath + ".old";
    QFile::remove(old);
    QFile::rename(filePath, old);
  }

  auto *file = new QFile(filePath);
  if (!file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    delete file;
    return false;
  }

  {
    QMutexLocker locker(&s_mutex);
    s_file = file;
    s_stream = new QTextStream(s_file);
  }
  s_previous = qInstallMessageHandler(messageHandler);
  s_installed = true;
  return true;
}

void shutdown() {
  if (s_installed) {
    qInstallMessageHandler(s_previous);
    s_previous = nullptr;
    s_installed = false;
  }
  QMutexLocker locker(&s_mutex);
  delete s_stream;
  s_stream = nullptr;
  delete s_file;
  s_file = nullptr;
}

bool isDebugEnabled() { return s_debugEnabled; }

} // namespace Logging
