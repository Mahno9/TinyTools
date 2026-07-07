#include <QtTest>
#include "../../src/app/Logging.h"
#include <QFile>
#include <QTemporaryDir>
#include <QThread>
#include <QVector>

// Tests the REAL logging module used by main.cpp (not a replica).

class LogWorker : public QThread {
public:
    explicit LogWorker(int id, int iterations, QObject *parent = nullptr)
        : QThread(parent), m_id(id), m_iterations(iterations) {}

protected:
    void run() override {
        for (int i = 0; i < m_iterations; ++i) {
            qWarning() << QString("Thread %1 message %2").arg(m_id).arg(i);
        }
    }

private:
    int m_id;
    int m_iterations;
};

class TestLogging : public QObject {
    Q_OBJECT

private slots:
    void cleanup();
    void testWritesToFile();
    void testDebugSuppressedByDefault();
    void testDebugEnabledByEnvVar();
    void testRotationOnOversizedFile();
    void testConcurrentLinesAreComplete();

private:
    static QString readAll(const QString &path) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
        return QString::fromUtf8(f.readAll());
    }
};

void TestLogging::cleanup() {
    Logging::shutdown();
    qunsetenv("TINYTOOLS_DEBUG");
}

void TestLogging::testWritesToFile() {
    QTemporaryDir dir;
    const QString path = dir.path() + "/log/app.log"; // subdir must be created
    QVERIFY(Logging::init(path));

    qInfo() << "hello info";
    qWarning() << "hello warning";
    Logging::shutdown();

    const QString content = readAll(path);
    QVERIFY(content.contains("[INFO] hello info"));
    QVERIFY(content.contains("[WARN] hello warning"));
}

void TestLogging::testDebugSuppressedByDefault() {
    qunsetenv("TINYTOOLS_DEBUG");
    QTemporaryDir dir;
    const QString path = dir.path() + "/app.log";
    QVERIFY(Logging::init(path));
    QVERIFY(!Logging::isDebugEnabled());

    qDebug() << "invisible debug line";
    qInfo() << "visible info line";
    Logging::shutdown();

    const QString content = readAll(path);
    QVERIFY(!content.contains("invisible debug line"));
    QVERIFY(content.contains("visible info line"));
}

void TestLogging::testDebugEnabledByEnvVar() {
    qputenv("TINYTOOLS_DEBUG", "1");
    QTemporaryDir dir;
    const QString path = dir.path() + "/app.log";
    QVERIFY(Logging::init(path));
    QVERIFY(Logging::isDebugEnabled());

    qDebug() << "now visible debug line";
    Logging::shutdown();

    QVERIFY(readAll(path).contains("now visible debug line"));
}

void TestLogging::testRotationOnOversizedFile() {
    QTemporaryDir dir;
    const QString path = dir.path() + "/app.log";

    // Create an oversized "previous run" log
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QByteArray(2048, 'x'));
    }

    QVERIFY(Logging::init(path, /*maxBytes=*/1024));
    qInfo() << "fresh line";
    Logging::shutdown();

    QVERIFY(QFile::exists(path + ".old"));
    const QString fresh = readAll(path);
    QVERIFY(fresh.contains("fresh line"));
    QVERIFY(!fresh.contains("xxxx")); // old content moved aside
}

void TestLogging::testConcurrentLinesAreComplete() {
    QTemporaryDir dir;
    const QString path = dir.path() + "/app.log";
    QVERIFY(Logging::init(path));

    const int THREAD_COUNT = 5;
    const int MSG_PER_THREAD = 100;

    QVector<LogWorker *> workers;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        workers.append(new LogWorker(i, MSG_PER_THREAD, this));
    }
    for (auto *w : workers) w->start();
    for (auto *w : workers) w->wait();
    qDeleteAll(workers);

    Logging::shutdown();

    const QStringList lines =
        readAll(path).split('\n', Qt::SkipEmptyParts);
    QCOMPARE(lines.size(), THREAD_COUNT * MSG_PER_THREAD);
    for (const QString &line : lines) {
        QVERIFY2(line.contains("Thread") && line.contains("[WARN]"),
                 qPrintable(QString("Torn/unexpected line: \"%1\"").arg(line)));
    }
}

QTEST_GUILESS_MAIN(TestLogging)
#include "test_logging.moc"
