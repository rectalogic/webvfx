#include <string.h>
#include <QtGlobal>
#include "webvfx/image.h"

namespace WebVfx
{

const int Image::BytesPerPixel;

void Image::copyPixels(const Image& sourceImage, Image& targetImage)
{
    if (targetImage.byteCount_ == sourceImage.byteCount_)
        memcpy(targetImage.pixels_, sourceImage.pixels_, targetImage.byteCount_);
    else {
        unsigned char* sourceP = sourceImage.pixels_;
        int sourceRowBytes = sourceImage.bytesPerLine();
        unsigned char* targetP = targetImage.pixels_;
        int targetRowBytes = targetImage.bytesPerLine();
        int widthBytes = targetImage.width_ * BytesPerPixel;
        for (int i = 0; i < targetImage.height_; i++) {
            memcpy(targetP, sourceP, widthBytes);
            sourceP += sourceRowBytes;
            targetP += targetRowBytes;
        }
    }
}

void Image::copyPixelsFrom(const Image& sourceImage) {
    Q_ASSERT(compatible(sourceImage));
    copyPixels(sourceImage, *this);
}

void Image::copyPixelsTo(Image& targetImage) const {
    Q_ASSERT(compatible(targetImage));
    copyPixels(*this, targetImage);
}

}
