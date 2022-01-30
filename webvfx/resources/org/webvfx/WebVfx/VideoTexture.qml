// Copyright (c) 2021 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import QtQuick
import QtQuick3D
import org.webvfx.WebVfx.native 1.0

Texture {
    id: video
    required property string imageName

    textureData: ImageTexture {
        id: texture
    }

    Component.onCompleted: {
        webvfx.registerImageName(video.imageName);
    }

    Connections {
        target: webvfx
        function onRenderRequested(time) {
    	    texture.image = webvfx.getImage(video.imageName);
        }
    }
}
