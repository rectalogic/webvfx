// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_IMAGE_H_
#define WEBVFX_IMAGE_H_

namespace WebVFX
{

// Image is just a way to pass around a reference to a buffer of image data,
// along with metadata about it (byte count, width, height etc.)
// The underlying data is not owned or managed by Image.
class Image
{
public:
    static const int BytesPerPixel = 3;

    Image() {}

    // pixels must be valid for the lifetime of the Image
    // pixels are always 24 bit RGB
    Image(unsigned char* pixels, int width, int height, int byteCount)
        : pixels_(pixels)
        , width_(width)
        , height_(height)
        , byteCount_(byteCount) {}

    unsigned char* pixels() { return pixels_; }
    const unsigned char* pixels() const { return pixels_; }
    int width() const { return width_; }
    int height() const { return height_; }
    int bytesPerLine() const { return byteCount_ / height_; }
    int byteCount() const { return byteCount_; }

    void copyPixelsFrom(const Image& sourceImage);
    void copyPixelsTo(Image& targetImage) const;
    bool compatible(const Image& image) const {
        return (image.pixels_ && pixels_
                && image.pixels_ != pixels_
                && image.width_ == width_
                && image.height_ == height_);
    }

private:
    static void copyPixels(const Image& sourceImage, Image& targetImage);

    unsigned char* pixels_;
    int width_;
    int height_;
    int byteCount_;
};

}

#endif
