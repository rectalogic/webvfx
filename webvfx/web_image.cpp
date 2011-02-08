#include <string.h>
#include <QtGlobal>
#include "webvfx/web_image.h"

namespace WebVFX
{

const int WebImage::BytesPerPixel;

void WebImage::copyPixels(const WebImage& sourceImage, WebImage& targetImage)
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

void WebImage::copyPixelsFrom(const WebImage& sourceImage) {
    Q_ASSERT(compatible(sourceImage));
    copyPixels(sourceImage, *this);
}

void WebImage::copyPixelsTo(WebImage& targetImage) const {
    Q_ASSERT(compatible(targetImage));
    copyPixels(*this, targetImage);
}

}
