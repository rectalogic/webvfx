// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <arpa/inet.h>
#include <cmath>

namespace WebVfxCommon {
    //! Number of bytes per pixel.
    static const int BytesPerPixel = 3;

    static const uint32_t NanoSeconds = 1000000000;

    inline uint32_t toTimecode(uint32_t position, uint32_t duration) {
        return htonl((uint32_t)round((position / (double)duration) * NanoSeconds));
    };

    inline double fromTimecode(uint32_t timecode) {
        return ntohl(timecode) / (double)NanoSeconds;
    };

}