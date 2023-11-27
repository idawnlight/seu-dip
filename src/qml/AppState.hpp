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
    void loadImage(const QString &imgPath, const QString& flag) const {
        int cv_flag = cv::IMREAD_UNCHANGED;
        if (flag == "color") {
            cv_flag = cv::IMREAD_COLOR;
        } else if (flag == "grayscale") {
            cv_flag = cv::IMREAD_GRAYSCALE;
        }

        if (imgPath != "") provider->loadImage(imgPath.toStdString(), cv_flag);
    }

    void blackImage(int width = 500, int height = 500) const {
        provider->blackImage(width, height);
    }

    void resetImage() const {
        provider->resetImage();
    }

    void swapImage() const {
        provider->swapImage();
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

    void gaussianNoise(double mean = 0, double stddev = 24) const {
        provider->gaussianNoise(mean, stddev);
    }

    void saltAndPepperNoise() const {
        provider->saltAndPepperNoise();
    }

    void medianFilter(int kernelSize = 3) const {
        provider->medianFilter(kernelSize);
    }

    void meanFilter(int kernelSize = 3) const {
        provider->meanFilter(kernelSize);
    }

    void adaptiveMedianFilter() const {
        provider->adaptiveMedianFilter();
    }

    void nonLocalMeanFilter() const {
        provider->nonLocalMeanFilter();
    }

    void laplacianSharpening() const {
        provider->laplacianSharpening();
    }

    void unsharpMasking(double sigma = 2.0, double k = 1) const {
        provider->unsharpMasking(sigma, k);
    }

    void adaptiveLocalNoiseReduction() const {
        provider->adaptiveLocalNoiseReduction();
    }

    void jpeg() const {
        provider->jpeg();
    }

    void jpeg2000() const {
        provider->jpeg2000();
    }

};

#endif //SEU_DIP_APPSTATE_HPP
