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
};

#endif //SEU_DIP_CUSTOMPROCESSOR_HPP
