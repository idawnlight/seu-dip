#ifndef SEU_DIP_OPENCVIMAGEPROVIDER_HPP
#define SEU_DIP_OPENCVIMAGEPROVIDER_HPP

#include <QQuickImageProvider>
#include "src/utility/OpenCVProcessor.hpp"

class OpenCVImageProvider : public QQuickImageProvider {
public:
    explicit OpenCVImageProvider(OpenCVProcessor *p) : QQuickImageProvider(QQuickImageProvider::Pixmap), processor(p) {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        if (id == "origin") {
            return OpenCVProcessor::cvtMat2Pixmap(processor->getOriginImage());
        } else if (id == "processed") {
            return OpenCVProcessor::cvtMat2Pixmap(processor->getProcessedImage());
        } else {
            return {};
        }
    }

private:
    OpenCVProcessor *processor;
};

#endif //SEU_DIP_OPENCVIMAGEPROVIDER_HPP
