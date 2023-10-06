#ifndef SEU_DIP_OPENCVPROCESSOR_HPP
#define SEU_DIP_OPENCVPROCESSOR_HPP

#include <opencv2/opencv.hpp>
#include <QPixmap>

class OpenCVProcessor {
public:
    static cv::Mat fourierTrans(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2GRAY);

        cv::Mat planes[] = { cv::Mat_<float>(processedImage), cv::Mat::zeros(processedImage.size(), CV_32F) };

        cv::Mat complexI;
        cv::merge(planes, 2, complexI);
        std::cout << complexI.size() << std::endl;
        std::cout << planes->size() << std::endl;

        cv::dft(complexI, complexI);

        split(complexI, planes);
        magnitude(planes[0], planes[1], planes[0]);
        cv::Mat magnitudeImage = planes[0];

        magnitudeImage += cv::Scalar::all(1);
        cv::log(magnitudeImage, magnitudeImage);

        magnitudeImage = magnitudeImage(cv::Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));

        int cx = magnitudeImage.cols / 2;
        int cy = magnitudeImage.rows / 2;

        cv::Mat q0(magnitudeImage, cv::Rect(0, 0, cx, cy));
        cv::Mat q1(magnitudeImage, cv::Rect(cx, 0, cx, cy));
        cv::Mat q2(magnitudeImage, cv::Rect(0, cy, cx, cy));
        cv::Mat q3(magnitudeImage, cv::Rect(cx, cy, cx, cy));

        cv::Mat tmp;
        q0.copyTo(tmp);
        q3.copyTo(q0);
        tmp.copyTo(q3);
        q1.copyTo(tmp);
        q2.copyTo(q1);
        tmp.copyTo(q2);

        cv::normalize(magnitudeImage, magnitudeImage, 0, 1, cv::NORM_MINMAX);
        magnitudeImage.convertTo(magnitudeImage, CV_8UC1, 255, 0);

        processedImage = magnitudeImage;
        return processedImage;
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
};


#endif //SEU_DIP_OPENCVPROCESSOR_HPP
