#include <QApplication>
#include <QQmlApplicationEngine>
#include "AppState.hpp"
#include "ImageProvider.hpp"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    const auto provider = new ImageProvider();
    QFile lena(":/dip/res/lena.bmp");
    lena.open(QIODevice::ReadOnly);
    QByteArray data = lena.readAll();
    provider->loadImage(std::vector<uchar>(data.begin(), data.end()));

    // Register OpenCV Image Provider
    engine.addImageProvider("cv", provider);

    engine.load(QUrl(u"qrc:/dip/qml/main.qml"_qs));

    // Locate Singleton of AppState
    AppState::typeId = qmlTypeId("dip", 1, 0, "AppState");
    auto *appState = engine.singletonInstance<AppState *>(AppState::typeId);

    appState->provider = provider;

    return QGuiApplication::exec();
}
