#ifndef SEU_DIP_IMAGEPROVIDER_HPP
#define SEU_DIP_IMAGEPROVIDER_HPP

#include <QQuickImageProvider>
#include "src/utility/OpenCVProcessor.hpp"

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

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        if (id.startsWith("origin")) {
            return OpenCVProcessor::cvtMat2Pixmap(getOriginImage());
        } else if (id.startsWith("processed")) {
            return OpenCVProcessor::cvtMat2Pixmap(getProcessedImage());
        } else {
            return {};
        }
    }

    void loadImage(const std::string& path) {
        originImage = cv::imread(path);
        processedImage = originImage.clone();
    }

    void loadImage(const std::vector<uchar>& data) {
        originImage = cv::imdecode(data, cv::IMREAD_COLOR);
        processedImage = originImage.clone();
    }

    void fourierTrans() {
        processedImage = OpenCVProcessor::fourierTrans(originImage);
    }

private:
    cv::Mat originImage;
    cv::Mat processedImage;
};

#endif //SEU_DIP_IMAGEPROVIDER_HPP
