#ifndef SEU_DIP_OPENCVPROCESSOR_HPP
#define SEU_DIP_OPENCVPROCESSOR_HPP

#include <opencv2/opencv.hpp>

class OpenCVProcessor {
public:
    static cv::Mat fourierTrans(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2GRAY);

        cv::Mat planes[] = {cv::Mat_<float>(processedImage), cv::Mat::zeros(processedImage.size(), CV_32F)};

        cv::Mat complexI;
        cv::merge(planes, 2, complexI);
//        std::cout << complexI.size() << std::endl;
//        std::cout << planes->size() << std::endl;

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

    static cv::Mat histogramEqualizationGray(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(processedImage, processedImage);
        return processedImage;
    }

    static cv::Mat histogramEqualization(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2YUV);
        std::vector<cv::Mat> channels;
        cv::split(processedImage, channels);
        cv::equalizeHist(channels[0], channels[0]);
        cv::merge(channels, processedImage);
        cv::cvtColor(processedImage, processedImage, cv::COLOR_YUV2BGR);
        return processedImage;
    }

    static cv::Mat applyCLAHEGray(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2GRAY);
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
        clahe->setClipLimit(4);
        clahe->apply(processedImage, processedImage);
        return processedImage;
    }

    static cv::Mat applyCLAHE(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2YUV);
        std::vector<cv::Mat> channels;
        cv::split(processedImage, channels);

        const cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
        clahe->setClipLimit(4);
        clahe->apply(channels[0], channels[0]);
        cv::merge(channels, processedImage);
        cv::cvtColor(processedImage, processedImage, cv::COLOR_YUV2BGR);
        return processedImage;
    }

    static cv::Mat medianFilter(const cv::Mat &originImage, int kernelSize = 3) {
        cv::Mat processedImage;
        cv::medianBlur(originImage, processedImage, kernelSize);
        return processedImage;
    }

    static cv::Mat meanFilter(const cv::Mat &originImage, int kernelSize = 3) {
        cv::Mat processedImage;
        cv::blur(originImage, processedImage, cv::Size(kernelSize, kernelSize));
        return processedImage;
    }

    static cv::Mat nonLocalMeanFilter(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::fastNlMeansDenoising(originImage, processedImage);
        return processedImage;
    }

    static cv::Mat drawHistogram(const cv::Mat &originImage) {
        cv::Mat grayImage;
        switch (originImage.type()) {
            case CV_8UC1:
                grayImage = originImage;
                break;
            case CV_8UC3:
                cv::cvtColor(originImage, grayImage, cv::COLOR_BGR2GRAY);
                break;
            case CV_8UC4:
                cv::cvtColor(originImage, grayImage, cv::COLOR_BGRA2GRAY);
                break;
            default:
                return {};
        }

        cv::Mat hist;
        int histSize = 256;
        float range[] = {0, 256};
        const float *histRange = {range};
        cv::calcHist(&grayImage, 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange);

        int hist_w = 1024;
        int hist_h = 256;
        int bin_w = cvRound((double) hist_w / histSize);
        cv::Mat histImage(hist_h, hist_w, CV_8UC1, cv::Scalar(0, 0, 0));

        // normalize to height of image
        cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

        for (int i = 0; i < histSize; i++) {
            if (cvRound(hist.at<float>(i) == 0)) continue;
            auto start_i = bin_w * (i - 1) < 0 ? 0 : bin_w * (i - 1);
            auto end_i = bin_w * i > hist_w ? hist_w : bin_w * i;
            for (int j = start_i; j < end_i; j++) {
                // draw line
                cv::line(histImage, cv::Point(j, hist_h),
                         cv::Point(j, hist_h - cvRound(hist.at<float>(i))),
                         cv::Scalar(255, 255, 255), 1);
            }
        }

        return histImage;
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
