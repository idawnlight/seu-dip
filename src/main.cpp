#include <QApplication>
#include <QPushButton>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/dip/qml/main.qml"_qs);
    engine.load(url);

    return QGuiApplication::exec();
}
