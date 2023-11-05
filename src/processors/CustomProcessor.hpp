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
        for (int i = 0; i < luma.rows; ++i) {
            for (int j = 0; j < luma.cols; ++j) {
                histogram[luma.at<uchar>(i, j)]++;
            }
        }

        // Calculate mapping
        int sum = 0;
        int *mapping = new int[256];
        for (int i = 0; i < 256; ++i) {
            sum += histogram[i];
            mapping[i] = sum * 255 / (luma.rows * luma.cols);
        }

        // Apply mapping
        for (int i = 0; i < luma.rows; ++i) {
            for (int j = 0; j < luma.cols; ++j) {
                luma.at<uchar>(i, j) = mapping[luma.at<uchar>(i, j)];
            }
        }

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
        cv::Mat processedImage;
        cv::cvtColor(originImage, processedImage, cv::COLOR_BGR2YUV);
        std::vector<cv::Mat> channels;
        cv::split(processedImage, channels);
        cv::Mat luma = channels[0];

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

        channels[0] = padded(cv::Rect(0, 0, luma.cols, luma.rows));
        cv::merge(channels, processedImage);
        cv::cvtColor(processedImage, processedImage, cv::COLOR_YUV2BGR);
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
        cv::copyMakeBorder(processedImage, padded, kernelSize / 2, kernelSize / 2, kernelSize / 2, kernelSize / 2,
                           cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                int ax = x + kernelSize / 2;
                int ay = y + kernelSize / 2;

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
        cv::copyMakeBorder(processedImage, padded, kernelSize / 2, kernelSize / 2, kernelSize / 2, kernelSize / 2,
                           cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                int ax = x + kernelSize / 2;
                int ay = y + kernelSize / 2;

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
        int maxSize = 7;

        cv::Mat processedImage = originImage.clone();
        cv::Mat padded;
        cv::copyMakeBorder(processedImage, padded, maxSize / 2, maxSize / 2, maxSize / 2, maxSize / 2,
                           cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                int ax = x + maxSize / 2;
                int ay = y + maxSize / 2;

                int size = 3;
                while (size <= maxSize) {
                    int block[size * size];
                    for (int i = 0; i < size; i++) {
                        for (int j = 0; j < size; j++) {
                            block[i * size + j] = padded.at<uchar>(ay + i - size / 2, ax + j - size / 2);
                        }
                    }

                    std::sort(block, block + size * size);
                    int median = block[size * size / 2];
                    int min = block[0];
                    int max = block[size * size - 1];

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
        int searchWindowSize = 7;
        int blockWindowSize = 3;
        double h = 10;

        cv::Mat processedImage = originImage.clone();
        cv::Mat padded;
        cv::copyMakeBorder(processedImage, padded, searchWindowSize / 2, searchWindowSize / 2, searchWindowSize / 2,
                           searchWindowSize / 2, cv::BORDER_REFLECT_101);

        for (int y = 0; y < processedImage.rows; y++) {
            for (int x = 0; x < processedImage.cols; x++) {
                double sum = 0.0;
                double totalWeight = 0.0;
                cv::Rect blockROI(x - blockWindowSize / 2, y - blockWindowSize / 2, blockWindowSize, blockWindowSize);

                for (int m = -searchWindowSize / 2; m <= searchWindowSize / 2; m++) {
                    for (int n = -searchWindowSize / 2; n <= searchWindowSize / 2; n++) {
                        cv::Rect searchROI(x + m - blockWindowSize / 2, y + n - blockWindowSize / 2, blockWindowSize,
                                           blockWindowSize);

                        // Calculate the weight for this block
                        double weight = 0.0;
                        for (int i = 0; i < blockWindowSize; i++) {
                            for (int j = 0; j < blockWindowSize; j++) {
                                double diff = padded.at<uchar>(blockROI.y + i, blockROI.x + j) - padded.at<uchar>(searchROI.y + i, searchROI.x + j);
                                weight += exp(-(diff * diff) / (2.0 * h * h));
                            }
                        }

                        // Update the sum and total weight
                        sum += weight * padded.at<uchar>(searchROI.y + blockWindowSize / 2, searchROI.x + blockWindowSize / 2);
                        totalWeight += weight;
                    }
                }

                // Update the pixel in the processed image
                processedImage.at<uchar>(y, x) = static_cast<uchar>(cvRound(sum / totalWeight));
            }
        }

        return processedImage;
    }
};

#endif //SEU_DIP_CUSTOMPROCESSOR_HPP
