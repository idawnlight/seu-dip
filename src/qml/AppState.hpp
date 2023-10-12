#ifndef SEU_DIP_APPSTATE_HPP
#define SEU_DIP_APPSTATE_HPP

#include <QObject>
#include <QtQmlIntegration>
#include <QFileDialog>
#include "ImageProvider.hpp"

class AppState : public QObject {
Q_OBJECT

    QML_ELEMENT
    QML_SINGLETON

public:
    AppState() = default;

    inline static int typeId;

    ImageProvider *provider = nullptr;

public slots:
    void loadImage(const QString &imgPath) const {
        if (imgPath != "")
            provider->loadImage(imgPath.toStdString());
    }

    void resetImage() const {
        provider->resetImage();
    }

    void fourierTrans() const {
        provider->fourierTrans();
    }

    void histogramEqualization(const QString& method = "YUV") const {
        provider->histogramEqualization(method);
    }

    void applyCLAHE(const QString& method = "YUV") const {
        provider->applyCLAHE(method);
    }

};

#endif //SEU_DIP_APPSTATE_HPP
