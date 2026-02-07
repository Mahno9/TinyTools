#include "app/Application.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

static QFile *logFile = nullptr;
static QTextStream *logStream = nullptr;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context,
                          const QString &msg) {
  QString timestamp =
      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
  QString level;

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

  QString logLine =
      QString("[%1] [%2] %3\n").arg(timestamp).arg(level).arg(msg);

  if (logStream) {
    *logStream << logLine;
    logStream->flush();
  }
}

int main(int argc, char *argv[]) {
  // Initialize logging BEFORE QApplication
  QString logPath = QDir::currentPath() + "/tinytools_log.txt";

  logFile = new QFile(logPath);
  if (logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
    logStream = new QTextStream(logFile);
    qInstallMessageHandler(customMessageHandler);

    qDebug() << "=== TinyTools Application Startup ===";
    qDebug() << "Log file:" << logFile->fileName();
  }

  qDebug() << "Entering main() function";
  qDebug() << "Arguments count:" << argc;

  try {
    qDebug() << "Creating QApplication...";
    QApplication app(argc, argv);
    // Prevent app from quitting when the last window is closed
    // This is essential for tray applications where the app should keep running
    // even when no windows are visible (e.g., settings dialog closed while main
    // window hidden)
    app.setQuitOnLastWindowClosed(false);
    qDebug() << "QApplication created successfully";
    qDebug() << "Application name:" << app.applicationName();
    qDebug() << "Application version:" << app.applicationVersion();
    qDebug() << "Organization name:" << app.organizationName();

    qDebug() << "Creating Application instance...";
    Application application;
    qDebug() << "Application instance created successfully";

    qDebug() << "Initializing application...";
    application.initialize();
    qDebug() << "Application initialized successfully";

    qDebug() << "Starting Qt event loop...";
    qDebug() << "=== Event loop started ===";
    int result = app.exec();
    qDebug() << "=== Event loop ended with code:" << result << "===";

    return result;
  } catch (const std::exception &e) {
    qCritical() << "FATAL: Unhandled exception in main():" << e.what();
    return 1;
  } catch (...) {
    qCritical() << "FATAL: Unknown exception in main()";
    return 1;
  }
}
