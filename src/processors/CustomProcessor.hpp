#ifndef SEU_DIP_CUSTOMPROCESSOR_HPP
#define SEU_DIP_CUSTOMPROCESSOR_HPP

#include <opencv2/opencv.hpp>

class CustomProcessor {
public:
    static cv::Mat histogramEqualizationGray(const cv::Mat &originImage) {
        assert(originImage.channels() == 1);

        cv::Mat luma = originImage;

        // Calculate histogram
        int histogram[256] = {0};
        for (int i = 0; i < luma.rows; i++) {
            for (int j = 0; j < luma.cols; ++j) {
                histogram[luma.at<uchar>(i, j)]++;
            }
        }

        // Calculate mapping
        int sum = 0;
        const auto mapping = new int[256];
        for (int i = 0; i < 256; i++) {
            sum += histogram[i];
            mapping[i] = sum * 255 / (luma.rows * luma.cols);
        }

        // Apply mapping
        for (int i = 0; i < luma.rows; i++) {
            for (int j = 0; j < luma.cols; j++) {
                luma.at<uchar>(i, j) = mapping[luma.at<uchar>(i, j)];
            }
        }

        delete[] mapping;

        return luma;
    }

    static cv::Mat histogramEqualization(const cv::Mat &originImage) {
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2YUV);
        std::vector<cv::Mat> channels;
        cv::split(processedImage, channels);

        channels[0] = histogramEqualizationGray(channels[0]);

        cv::merge(channels, processedImage);
        cv::cvtColor(processedImage, processedImage, cv::COLOR_YUV2BGR);
        return processedImage;
    }

    static cv::Mat CLAHE(const cv::Mat &originImage, int tilesX = 8, int tilesY = 8) {
        cv::Mat processedImage = originImage.clone();
        cv::Mat luma;
        std::vector<cv::Mat> channels;
        if (originImage.channels() == 3) {
            cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2YUV);
            cv::split(processedImage, channels);
            luma = channels[0];
        } else {
            luma = processedImage;
        }

        int mapping[tilesX * tilesY][256];
        cv::Size tileSize;
        cv::Mat padded;

        if (luma.cols % tilesX == 0 && luma.rows % tilesY == 0) {
            tileSize = cv::Size(luma.cols / tilesX, luma.rows / tilesY);
            padded = luma;
        } else {
            cv::copyMakeBorder(luma, padded, 0, tilesY - (luma.rows % tilesY), 0, tilesX - (luma.cols % tilesX),
                               cv::BORDER_REFLECT_101);
            tileSize = cv::Size(padded.cols / tilesX, padded.rows / tilesY);
        }

        // split into tiles
        for (int i = 0; i < tilesX; i++) {
            for (int j = 0; j < tilesY; j++) {
                cv::Mat tile = padded(cv::Rect(i * tileSize.width, j * tileSize.height, tileSize.width, tileSize.height));

                // calculate histogram
                int histogram[256] = {0};
                for (int k = 0; k < tile.rows; ++k) {
                    for (int l = 0; l < tile.cols; ++l) {
                        histogram[tile.at<uchar>(k, l)]++;
                    }
                }

                // clip
                int clipLimit = std::max(1, (int) (4 * (tile.rows * tile.cols / 256.0)));
                int clipped = 0;
                for (int & k : histogram) {
                    if (k > clipLimit) {
                        clipped += k - clipLimit;
                        k = clipLimit;
                    }
                }
                int increment = clipped / 256;
                for (int & k : histogram) {
                    k += increment;
                }

                // final mapping / lut for tile
                int sum = 0;
                for (int k = 0; k < 256; ++k) {
                    sum += histogram[k];
                    mapping[i * tilesX + j][k] = sum * 255 / (tile.rows * tile.cols);
                }
            }
        }

        // bilinear interpolation
        for (int y = 0; y < padded.rows; y++) {
            for (int x = 0; x < padded.cols; x++) {
                int ax = x - (tileSize.width / 2);
                int ay = y - (tileSize.height / 2);

                int blocks[4][2], blockId[4];
                blocks[0][0] = std::floor((double) ax / tileSize.width);
                blocks[0][1] = std::floor((double) ay / tileSize.height);
                blocks[1][0] = blocks[0][0] + 1;
                blocks[1][1] = blocks[0][1];
                blocks[2][0] = blocks[0][0];
                blocks[2][1] = blocks[0][1] + 1;
                blocks[3][0] = blocks[0][0] + 1;
                blocks[3][1] = blocks[0][1] + 1;

                for (int i = 0; i < 4; i++) {
                    auto &block = blocks[i];
                    if (block[0] < 0) block[0] = 0;
                    if (block[1] < 0) block[1] = 0;
                    if (block[0] >= tilesX) block[0] = tilesX - 1;
                    if (block[1] >= tilesY) block[1] = tilesY - 1;

                    blockId[i] = block[0] * tilesX + block[1];
                }

                int e1 = mapping[blockId[0]][padded.at<uchar>(y, x)];
                int e2 = mapping[blockId[1]][padded.at<uchar>(y, x)];
                int e3 = mapping[blockId[2]][padded.at<uchar>(y, x)];
                int e4 = mapping[blockId[3]][padded.at<uchar>(y, x)];

                int e12 = e1 + (e2 - e1) * (ax % tileSize.width) / tileSize.width;
                int e34 = e3 + (e4 - e3) * (ax % tileSize.width) / tileSize.width;
                int e = e12 + (e34 - e12) * (ay % tileSize.height) / tileSize.height;

                padded.at<uchar>(y, x) = e;
            }
        }

        cv::Mat result = padded(cv::Rect(0, 0, luma.cols, luma.rows));
        if (originImage.channels() == 3) {
            channels[0] = padded(cv::Rect(0, 0, luma.cols, luma.rows));
            cv::merge(channels, processedImage);
            cv::cvtColor(processedImage, processedImage, cv::COLOR_YUV2BGR);
        } else {
            processedImage = result;
        }
        return processedImage;
    }

    static cv::Mat gaussianNoise(const cv::Mat &originImage, double mean = 0, double stddev = 10) {
        cv::Mat processedImage = originImage.clone();
        cv::Mat noise = cv::Mat(processedImage.size(), processedImage.type());
        cv::randn(noise, mean, stddev);
        cv::add(processedImage, noise, processedImage);
        return processedImage;
    }

    static cv::Mat saltAndPepperNoise(const cv::Mat &originImage) {
        cv::Mat processedImage = originImage.clone();
        if (processedImage.channels() == 3) cv::cvtColor(processedImage, processedImage, cv::COLOR_BGR2GRAY);
        cv::Mat noise = cv::Mat(processedImage.size(), CV_8UC1);
        cv::randu(noise, 0, 255);
        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                if (noise.at<uchar>(y, x) < 20) {
                    processedImage.at<uchar>(y, x) = 0;
                } else if (noise.at<uchar>(y, x) > 235) {
                    processedImage.at<uchar>(y, x) = 255;
                }
            }
        }
        return processedImage;
    }

    static cv::Mat medianFilter(const cv::Mat &originImage, int kernelSize = 3) {
        cv::Mat processedImage = originImage.clone();
        cv::Mat padded;
        const int padding = kernelSize / 2;
        cv::copyMakeBorder(processedImage, padded, padding, padding, padding, padding,
                           cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                const int ax = x + padding;
                const int ay = y + padding;

                int block[kernelSize * kernelSize];
                for (int i = 0; i < kernelSize; i++) {
                    for (int j = 0; j < kernelSize; j++) {
                        block[i * kernelSize + j] = padded.at<uchar>(ay + i - kernelSize / 2, ax + j - kernelSize / 2);
                    }
                }

                std::sort(block, block + kernelSize * kernelSize);
                processedImage.at<uchar>(y, x) = block[kernelSize * kernelSize / 2];
            }
        }

        return processedImage;
    }

    static cv::Mat meanFilter(const cv::Mat &originImage, int kernelSize = 3) {
        cv::Mat processedImage = originImage.clone();
        cv::Mat padded;
        const int padding = kernelSize / 2;
        cv::copyMakeBorder(processedImage, padded, padding, padding, padding, padding,
                           cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                const int ax = x + padding;
                const int ay = y + padding;

                int sum = 0;
                for (int i = 0; i < kernelSize; i++) {
                    for (int j = 0; j < kernelSize; j++) {
                        sum += padded.at<uchar>(ay + i - kernelSize / 2, ax + j - kernelSize / 2);
                    }
                }

                processedImage.at<uchar>(y, x) = sum / (kernelSize * kernelSize);
            }
        }

        return processedImage;
    }

    static cv::Mat adaptiveMedianFilter(const cv::Mat &originImage) {
        const int maxSize = 7;

        cv::Mat processedImage = originImage.clone();
        cv::Mat padded;
        cv::copyMakeBorder(processedImage, padded, maxSize / 2, maxSize / 2, maxSize / 2, maxSize / 2,
                           cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                const int ax = x + maxSize / 2;
                const int ay = y + maxSize / 2;

                int size = 3;
                while (size <= maxSize) {
                    int block[size * size];
                    for (int i = 0; i < size; i++) {
                        for (int j = 0; j < size; j++) {
                            block[i * size + j] = padded.at<uchar>(ay + i - size / 2, ax + j - size / 2);
                        }
                    }

                    std::sort(block, block + size * size);
                    const int median = block[size * size / 2];
                    const int min = block[0];
                    const int max = block[size * size - 1];

                    if (min < median && median < max) {
                        if (min >= processedImage.at<uchar>(y, x) || processedImage.at<uchar>(y, x) >= max) {
                            processedImage.at<uchar>(y, x) = median;
                        }
                        break;
                    }

                    size += 2;
                }
            }
        }

        return processedImage;
    }

    static cv::Mat nonLocalMeanFilter(const cv::Mat &originImage) {
        const int halfSearchSize = 7;
        const int halfBlockSize = 3;
        const int padding = halfSearchSize + halfBlockSize;
        const double h = 10;

        cv::Mat processedImage = originImage.clone();
        cv::Mat padded;
        cv::copyMakeBorder(processedImage, padded, padding, padding, padding,
                           padding, cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                const cv::Rect currentBlock(x - halfBlockSize + padding, y - halfBlockSize + padding, 2 * halfBlockSize + 1, 2 * halfBlockSize + 1);

                double p = 0, w = 0;

                for (int i = -halfSearchSize; i <= halfSearchSize; i++) {
                    for (int j = -halfSearchSize; j <= halfSearchSize; j++) {
                        const cv::Rect searchBlock(x + j - halfBlockSize + padding, y + i - halfBlockSize + padding, 2 * halfBlockSize + 1, 2 * halfBlockSize + 1);

                        double d = 0;
                        for (int k = 0; k < 2 * halfBlockSize + 1; k++) {
                            for (int l = 0; l < 2 * halfBlockSize + 1; l++) {
                                d += std::pow(padded.at<uchar>(currentBlock.y + k, currentBlock.x + l) - padded.at<uchar>(searchBlock.y + k, searchBlock.x + l), 2);
                            }
                        }
                        d /= (2 * halfBlockSize + 1) * (2 * halfBlockSize + 1);

                        const double w_i = std::exp(-std::max(d - 2 * h * h, 0.0) / (h * h));
                        w += w_i;
                        p += w_i * padded.at<uchar>(searchBlock.y + halfBlockSize, searchBlock.x + halfBlockSize);
                    }
                }

                processedImage.at<uchar>(y, x) = p / w;
            }
        }

        return processedImage;
    }

    static cv::Mat laplacianSharpening(const cv::Mat &originImage) {
        cv::Mat padded, laplacian(originImage.size(), CV_8UC1);
        cv::copyMakeBorder(originImage, padded, 1, 1, 1, 1, cv::BORDER_REFLECT_101);

        for (int y = 0; y < originImage.rows; y++) {
            for (int x = 0; x < originImage.cols; x++) {
                const int ax = x + 1;
                const int ay = y + 1;

                int sum = padded.at<uchar>(ay - 1, ax - 1) + padded.at<uchar>(ay - 1, ax) + padded.at<uchar>(ay - 1, ax + 1) +
                          padded.at<uchar>(ay, ax - 1) + padded.at<uchar>(ay, ax + 1) + padded.at<uchar>(ay + 1, ax - 1) +
                          padded.at<uchar>(ay + 1, ax) + padded.at<uchar>(ay + 1, ax + 1) - 8 * padded.at<uchar>(ay, ax);

                laplacian.at<uchar>(y, x) = std::max(0, std::min(255, sum));
            }
        }

        return originImage - laplacian;
    }

    static cv::Mat unsharpMasking(const cv::Mat &originImage, double sigma = 5, double k = 1) {
        cv::Mat blurred = originImage.clone();

        cv::GaussianBlur(blurred, blurred, cv::Size(5, 5), sigma);

        return originImage + k * (originImage - blurred);
    }

    static cv::Mat adaptiveLocalNoiseReduction(const cv::Mat &originImage, int kernelSize = 7) {
        cv::Mat padded, processedImage = originImage.clone();
        cv::copyMakeBorder(originImage, padded, 2, 2, 2, 2, cv::BORDER_REFLECT_101);

        const double sigmaEta = 36;

        for (int y = 0; y < originImage.rows; y++) {
            for (int x = 0; x < originImage.cols; x++) {
                const int ax = x + 2;
                const int ay = y + 2;

                // calculate mean and variance
                double mean = 0, variance = 0;
                for (int i = 0; i < kernelSize; i++) {
                    for (int j = 0; j < kernelSize; j++) {
                        mean += padded.at<uchar>(ay + i - kernelSize / 2, ax + j - kernelSize / 2);
                    }
                }
                mean /= kernelSize * kernelSize;
                for (int i = 0; i < kernelSize; i++) {
                    for (int j = 0; j < kernelSize; j++) {
                        variance += std::pow(padded.at<uchar>(ay + i - kernelSize / 2, ax + j - kernelSize / 2) - mean, 2);
                    }
                }
                variance /= kernelSize * kernelSize;

                if (sigmaEta / variance <= 1)
                    processedImage.at<uchar>(y, x) = std::max(0, std::min(255, (int) (
                            processedImage.at<uchar>(y, x) -
                                sigmaEta / variance * (processedImage.at<uchar>(y, x) - mean))));
            }
        }

        return processedImage;
    }

    static cv::Mat morphologyErosion(const cv::Mat &originImage, const cv::Mat &structuringElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3))) {
        cv::Mat processedImage = cv::Mat(originImage.size(), CV_8UC1, cv::Scalar(0));

        for (int y = 0; y < originImage.rows - structuringElement.rows / 2; y++) {
            for (int x = 0; x < originImage.cols - structuringElement.cols / 2; x++) {
                bool flag = true;
                for (int i = 0; i < structuringElement.rows; i++) {
                    for (int j = 0; j < structuringElement.cols; j++) {
                        if (structuringElement.at<uchar>(i, j) == 0) continue;
                        if (originImage.at<uchar>(y + i, x + j) == 0) {
                            flag = false;
                            break;
                        }
                    }
                    if (!flag) break;
                }

                processedImage.at<uchar>(y + 1, x + 1) = flag ? 255 : 0;
            }
        }

        return processedImage;
    }

    static cv::Mat morphologyDilation(const cv::Mat &originImage, const cv::Mat &structuringElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3))) {
        cv::Mat processedImage = cv::Mat(originImage.size(), CV_8UC1, cv::Scalar(0));

        for (int y = 0; y < originImage.rows - structuringElement.rows / 2; y++) {
            for (int x = 0; x < originImage.cols - structuringElement.cols / 2; x++) {
                bool flag = false;
                for (int i = 0; i < structuringElement.rows; i++) {
                    for (int j = 0; j < structuringElement.cols; j++) {
                        if (structuringElement.at<uchar>(i, j) == 0) continue;
                        if (originImage.at<uchar>(y + i, x + j) == 255) {
                            flag = true;
                            break;
                        }
                    }
                    if (flag) break;
                }

                processedImage.at<uchar>(y + 1, x + 1) = flag ? 255 : 0;
            }
        }

        return processedImage;
    }

    static cv::Mat morphologyOpening(const cv::Mat &originImage) {
        cv::Mat structuringElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::Mat processedImage = morphologyErosion(originImage, structuringElement);
        processedImage = morphologyDilation(processedImage, structuringElement);

        return processedImage;
    }

    static cv::Mat morphologySpecialForFingerprint(const cv::Mat &originImage) {
        cv::Mat processedImage = morphologyErosion(originImage, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)));
        processedImage = morphologyDilation(processedImage, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4)));
        processedImage = morphologyErosion(processedImage, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)));
        processedImage = morphologyDilation(processedImage, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)));

        return processedImage;
    }

    static cv::Mat morphologyClosing(const cv::Mat &originImage) {
        cv::Mat structuringElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::Mat processedImage = morphologyDilation(originImage, structuringElement);
        processedImage = morphologyErosion(processedImage, structuringElement);

        return processedImage;
    }

    static cv::Mat globalThresholding(const cv::Mat &originImage) {
        int threshold = cv::mean(originImage)[0];
        int t0 = 5;

        while (true) {
            int m0 = 0, m1 = 0, n0 = 0, n1 = 0;
            for (int y = 0; y < originImage.rows; y++) {
                for (int x = 0; x < originImage.cols; x++) {
                    if (originImage.at<uchar>(y, x) < threshold) {
                        m0 += originImage.at<uchar>(y, x);
                        n0++;
                    } else {
                        m1 += originImage.at<uchar>(y, x);
                        n1++;
                    }
                }
            }
            int t1 = (m0 / n0 + m1 / n1) / 2;
            if (std::abs(t1 - t0) < 1) break;
            t0 = t1;
        }

        cv::Mat processedImage = cv::Mat(originImage.size(), CV_8UC1, cv::Scalar(0));
        for (int y = 0; y < originImage.rows; y++) {
            for (int x = 0; x < originImage.cols; x++) {
                processedImage.at<uchar>(y, x) = originImage.at<uchar>(y, x) < threshold ? 0 : 255;
            }
        }

        return processedImage;
    }

    static cv::Mat otsuThresholding(const cv::Mat &originImage) {
        double histogram[256] = {0};
        for (int y = 0; y < originImage.rows; y++) {
            for (int x = 0; x < originImage.cols; x++) {
                histogram[originImage.at<uchar>(y, x)]++;
            }
        }
        for (double & i : histogram) {
            i /= originImage.rows * originImage.cols;
        }

        double max = 0;
        int threshold = 0;
        for (int i = 0; i < 256; i++) {
            double w0 = 0, w1 = 0, u0 = 0, u1 = 0, u = 0;
            for (int j = 0; j < 256; j++) {
                if (j <= i) {
                    w0 += histogram[j];
                    u0 += j * histogram[j];
                } else {
                    w1 += histogram[j];
                    u1 += j * histogram[j];
                }
            }

            u0 /= w0;
            u1 /= w1;

            u = w0 * w1 * std::pow(u0 - u1, 2);
            if (u > max) {
                max = u;
                threshold = i;
            }
        }

        cv::Mat processedImage = cv::Mat(originImage.size(), CV_8UC1, cv::Scalar(0));
        for (int y = 0; y < originImage.rows; y++) {
            for (int x = 0; x < originImage.cols; x++) {
                processedImage.at<uchar>(y, x) = originImage.at<uchar>(y, x) < threshold ? 0 : 255;
            }
        }

        return processedImage;
    }

};
#endif //SEU_DIP_CUSTOMPROCESSOR_HPP
