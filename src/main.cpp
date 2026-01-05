#include <QApplication>
#include <QLabel>
#include <QDebug>

int main(int argc, char* argv[]) {
    qDebug() << "Starting application...";
    
    QApplication app(argc, argv);
    qDebug() << "QApplication created";
    
    QLabel label("Yandex Translator");
    label.setAlignment(Qt::AlignCenter);
    label.resize(400, 300);
    label.show();
    qDebug() << "Label shown, starting event loop...";
    
    qDebug() << "App running successfully!";
    return app.exec();
}
