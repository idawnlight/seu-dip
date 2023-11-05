#ifndef SEU_DIP_IMAGEPROVIDER_HPP
#define SEU_DIP_IMAGEPROVIDER_HPP

#include <QQuickImageProvider>
#include "../processors/OpenCVProcessor.hpp"
#include "../processors/CustomProcessor.hpp"

class ImageProvider : public QQuickImageProvider {
public:
    explicit ImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    [[nodiscard]] const cv::Mat &getOriginImage() const {
        return originImage;
    }

    [[nodiscard]] const cv::Mat &getProcessedImage() const {
        return processedImage;
    }

    void resetImage() {
        processedImage = originImage.clone();
    }

    void swapImage() {
        originImage = processedImage.clone();
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        if (id.startsWith("histogram/origin")) {
            return OpenCVProcessor::cvtMat2Pixmap(OpenCVProcessor::drawHistogram(getOriginImage()));
        } else if (id.startsWith("histogram/processed")) {
            return OpenCVProcessor::cvtMat2Pixmap(OpenCVProcessor::drawHistogram(getProcessedImage()));
        } else if (id.startsWith("origin")) {
            return OpenCVProcessor::cvtMat2Pixmap(getOriginImage());
        } else if (id.startsWith("processed")) {
            return OpenCVProcessor::cvtMat2Pixmap(getProcessedImage());
        } else {
            return {};
        }
    }

    void loadImage(const std::string& path, int flag = cv::IMREAD_COLOR) {
        originImage = cv::imread(path, flag);
        processedImage = originImage.clone();
    }

    void loadImage(const std::vector<uchar>& data, int flag = cv::IMREAD_COLOR) {
        originImage = cv::imdecode(data, flag);
        processedImage = originImage.clone();
    }

    void blackImage(int width = 100, int height = 100) {
        originImage = cv::Mat::zeros(width, height, CV_8UC1);
        processedImage = originImage.clone();
    }

    void fourierTrans() {
        processedImage = OpenCVProcessor::fourierTrans(originImage);
    }

    void histogramEqualization(const QString& method = "YUV") {
        if (method == "YUV")
            processedImage = OpenCVProcessor::histogramEqualization(originImage);
        else if (method == "Gray")
            processedImage = OpenCVProcessor::histogramEqualizationGray(originImage);
        else if (method == "Custom")
            processedImage = CustomProcessor::histogramEqualization(originImage);
    }

    void applyCLAHE(const QString& method = "YUV") {
        if (method == "YUV")
            processedImage = OpenCVProcessor::applyCLAHE(originImage);
        else if (method == "Gray")
            processedImage = OpenCVProcessor::applyCLAHEGray(originImage);
        else if (method == "Custom")
            processedImage = CustomProcessor::CLAHE(originImage);
    }

    void gaussianNoise(double mean = 0, double stddev = 10) {
        processedImage = CustomProcessor::gaussianNoise(originImage, mean, stddev);
    }

    void saltAndPepperNoise() {
        processedImage = CustomProcessor::saltAndPepperNoise(originImage);
    }

    void medianFilter(int kernelSize = 3) {
        processedImage = CustomProcessor::medianFilter(originImage, kernelSize);
    }

    void meanFilter(int kernelSize = 3) {
        processedImage = CustomProcessor::meanFilter(originImage, kernelSize);
    }

    void adaptiveMedianFilter() {
        processedImage = CustomProcessor::adaptiveMedianFilter(originImage);
    }

    void nonLocalMeanFilter() {
        processedImage = CustomProcessor::nonLocalMeanFilter(originImage);
    }

private:
    cv::Mat originImage;
    cv::Mat processedImage;
};

#endif //SEU_DIP_IMAGEPROVIDER_HPP
