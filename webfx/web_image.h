// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_IMAGE_H_
#define WEBFX_WEB_IMAGE_H_

namespace WebFX
{

class WebImage
{
public:
    WebImage() {};

    // pixels must be valid for the lifetime of the WebImage
    // pixels are always 24 bit RGB
    WebImage(unsigned char* pixels, int width, int height, int byteCount)
        : pixels_(pixels)
        , width_(width)
        , height_(height)
        , byteCount_(byteCount) {};

    unsigned char* pixels() { return pixels_; }
    const unsigned char* pixels() const { return pixels_; }
    int width() const { return width_; }
    int height() const { return height_; }
    int bytePerLine() const { return byteCount_ / height_; }
    int byteCount() const { return byteCount_; }

    //XXX add copyPixels to/from methods

private:
    unsigned char* pixels_;
    int width_;
    int height_;
    int byteCount_;
};

}

#endif
