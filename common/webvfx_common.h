// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <stdint.h>

namespace WebVfxCommon {
    //! Number of bytes per pixel.
    static const int BytesPerPixel = 3;

    struct Timecode {
        Timecode(uint32_t position, uint32_t duration) : position(position), duration(duration) {}
        uint32_t position;
        uint32_t duration;

        inline double toDouble() {
            return this->position / (double)this->duration;
        };
    };


}