#include <QtTest>
#include <QMutex>
#include <QTextStream>
#include <QThread>
#include <QVector>
#include <atomic>

// ── Replicated log handler (mirrors src/main.cpp) ────────────────────────────
// We instantiate the same pattern here to verify thread-safety independently
// of the real app binary.

static QMutex   s_testLogMutex;
static QString  s_logBuffer;

static void testMessageHandler(QtMsgType, const QMessageLogContext &,
                               const QString &msg) {
    QMutexLocker locker(&s_testLogMutex);
    s_logBuffer += msg + "\n";
}
// ─────────────────────────────────────────────────────────────────────────────

class LogWorker : public QThread {
public:
    explicit LogWorker(int id, int iterations, QObject *parent = nullptr)
        : QThread(parent), m_id(id), m_iterations(iterations) {}

protected:
    void run() override {
        for (int i = 0; i < m_iterations; ++i) {
            qDebug() << QString("Thread %1 message %2").arg(m_id).arg(i);
        }
    }

private:
    int m_id;
    int m_iterations;
};

class TestLogging : public QObject {
    Q_OBJECT

private slots:
    void testConcurrentLoggingDoesNotCrash();
    void testEachLineIsComplete();
};

void TestLogging::testConcurrentLoggingDoesNotCrash() {
    // Install the thread-safe handler
    QtMessageHandler previous = qInstallMessageHandler(testMessageHandler);

    const int THREAD_COUNT  = 5;
    const int MSG_PER_THREAD = 100;

    QVector<LogWorker *> workers;
    workers.reserve(THREAD_COUNT);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        workers.append(new LogWorker(i, MSG_PER_THREAD, this));
    }

    for (auto *w : workers) w->start();
    for (auto *w : workers) w->wait();
    qDeleteAll(workers);

    qInstallMessageHandler(previous);

    // If we reach here without crashing, the mutex protected the writes.
    QVERIFY(true);
}

void TestLogging::testEachLineIsComplete() {
    s_logBuffer.clear();
    QtMessageHandler previous = qInstallMessageHandler(testMessageHandler);

    const int THREAD_COUNT  = 3;
    const int MSG_PER_THREAD = 50;

    QVector<LogWorker *> workers;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        workers.append(new LogWorker(i, MSG_PER_THREAD, this));
    }
    for (auto *w : workers) w->start();
    for (auto *w : workers) w->wait();
    qDeleteAll(workers);

    qInstallMessageHandler(previous);

    // Every line in the buffer must be non-empty and contain "Thread"
    QStringList lines = s_logBuffer.split('\n', Qt::SkipEmptyParts);
    QCOMPARE(lines.size(), THREAD_COUNT * MSG_PER_THREAD);
    for (const QString &line : lines) {
        QVERIFY2(line.contains("Thread"), qPrintable(
            QString("Unexpected line: \"%1\"").arg(line)));
    }
}

QTEST_GUILESS_MAIN(TestLogging)
#include "test_logging.moc"
