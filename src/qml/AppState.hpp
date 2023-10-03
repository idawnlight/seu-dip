#ifndef SEU_DIP_APPSTATE_HPP
#define SEU_DIP_APPSTATE_HPP

#include <QObject>
#include <QtQmlIntegration>
#include "src/utility/OpenCVProcessor.hpp"

class AppState : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString imgPath MEMBER imgPath NOTIFY imgPathChanged)

public:
    AppState() = default;

    inline static int typeId;

    QString imgPath = "/test.bmp";
    OpenCVProcessor* processor;

signals:
    void imgPathChanged();

};

#endif //SEU_DIP_APPSTATE_HPP
