#include <QApplication>
#include <QPushButton>
#include <QQmlApplicationEngine>
#include "qml/AppState.hpp"
#include "OpenCVImageProvider.hpp"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    auto processor = new OpenCVProcessor();
    QFile lena(":/dip/res/lena.bmp");
    lena.open(QIODevice::ReadOnly);
    QByteArray data = lena.readAll();
    processor->loadImage(std::vector<uchar>(data.begin(), data.end()));

    // Register OpenCV Image Provider
    engine.addImageProvider("cv", new OpenCVImageProvider(processor));

    engine.load(QUrl(u"qrc:/dip/qml/main.qml"_qs));

    // Locate Singleton of AppState
    AppState::typeId = qmlTypeId("dip", 1, 0, "AppState");
    auto *appState = engine.singletonInstance<AppState *>(AppState::typeId);

    appState->processor = processor;

    return QGuiApplication::exec();
}
