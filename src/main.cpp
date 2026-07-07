#include "app/Application.h"
#include "app/Constants.h"
#include "app/Logging.h"
#include <QApplication>
#include <QDebug>
#include <QStandardPaths>

int main(int argc, char *argv[]) {
  // Identity must be set before QStandardPaths is used anywhere, otherwise
  // the settings/log/WebEngine paths silently depend on the exe file name.
  // Organization name is deliberately NOT set: it would change
  // AppDataLocation and orphan existing user settings.
  QCoreApplication::setApplicationName(Constants::APP_NAME);
  QCoreApplication::setApplicationVersion(Constants::APP_VERSION);

  const QString dataDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  Logging::init(dataDir + "/tinytools.log");

  qInfo() << "=== TinyTools" << Constants::APP_VERSION << "startup ===";

  try {
    QApplication app(argc, argv);
    // Tray application: keep running when all windows are closed/hidden.
    app.setQuitOnLastWindowClosed(false);

    Application application;
    application.initialize();

    const int result = app.exec();
    qInfo() << "Event loop ended with code" << result;
    Logging::shutdown();
    return result;
  } catch (const std::exception &e) {
    qCritical() << "FATAL: Unhandled exception in main():" << e.what();
    Logging::shutdown();
    return 1;
  } catch (...) {
    qCritical() << "FATAL: Unknown exception in main()";
    Logging::shutdown();
    return 1;
  }
}
