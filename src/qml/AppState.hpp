#ifndef SEU_DIP_APPSTATE_HPP
#define SEU_DIP_APPSTATE_HPP

#include <QObject>
#include <QtQmlIntegration>
#include <QFileDialog>
#include "src/utility/OpenCVProcessor.hpp"

class AppState : public QObject {
Q_OBJECT

    QML_ELEMENT
    QML_SINGLETON

public:
    AppState() = default;

    inline static int typeId;

    OpenCVProcessor *processor;

public slots:
    void loadImage(const QString &imgPath) const {
        if (imgPath != "")
            processor->loadImage(imgPath.toStdString());
    }

    void resetImage() const {
        processor->resetImage();
    }

    void fourierTrans() const {
        processor->fourierTrans();
    }

};

#endif //SEU_DIP_APPSTATE_HPP
