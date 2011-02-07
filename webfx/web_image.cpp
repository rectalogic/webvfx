#include <string.h>
#include <QtGlobal>
#include "webfx/web_image.h"

namespace WebFX
{

const int WebImage::BytesPerPixel;

void WebImage::copyPixels(const WebImage& srcImage, WebImage& dstImage)
{
    if (dstImage.byteCount_ == srcImage.byteCount_)
        memcpy(dstImage.pixels_, srcImage.pixels_, dstImage.byteCount_);
    else {
        unsigned char* srcP = srcImage.pixels_;
        int srcRowBytes = srcImage.bytesPerLine();
        unsigned char* dstP = dstImage.pixels_;
        int dstRowBytes = dstImage.bytesPerLine();
        int widthBytes = dstImage.width_ * BytesPerPixel;
        for (int i = 0; i < dstImage.height_; i++) {
            memcpy(dstP, srcP, widthBytes);
            srcP += srcRowBytes;
            dstP += dstRowBytes;
        }
    }
}

void WebImage::copyPixelsFrom(const WebImage& sourceImage) {
    Q_ASSERT(compatible(sourceImage));
    copyPixels(sourceImage, *this);
}

void WebImage::copyPixelsTo(WebImage& destinationImage) const {
    Q_ASSERT(compatible(destinationImage));
    copyPixels(*this, destinationImage);
}

}
