#ifndef SEU_DIP_OPENCVPROCESSOR_HPP
#define SEU_DIP_OPENCVPROCESSOR_HPP

#include <opencv2/opencv.hpp>
#include <QPixmap>

class OpenCVProcessor {
public:
    OpenCVProcessor() = default;

    [[nodiscard]] const cv::Mat &getOriginImage() const {
        return originImage;
    };

    [[nodiscard]] const cv::Mat &getProcessedImage() const {
        return processedImage;
    };

    void loadImage(const std::string& path) {
        originImage = cv::imread(path);
        processedImage = originImage.clone();
    }

    void loadImage(const std::vector<uchar>& data) {
        originImage = cv::imdecode(data, cv::IMREAD_COLOR);
        processedImage = originImage.clone();
    }

    static QPixmap cvtMat2Pixmap(const cv::Mat &mat) {
        QImage img;
        switch (mat.type()) {
            case CV_8UC1:
                img = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
                break;
            case CV_8UC3:
                img = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
                img = img.rgbSwapped();
                break;
            case CV_8UC4:
                img = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
                break;
            default:
                return {};
        }

        QPixmap pixmap = QPixmap::fromImage(img);
        return pixmap;
    }
private:
    cv::Mat originImage;
    cv::Mat processedImage;
};


#endif //SEU_DIP_OPENCVPROCESSOR_HPP
