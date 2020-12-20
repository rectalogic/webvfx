// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import QtQuick

Image {
    id: video
    required property string imageName
    required property int imageType

    Component.onCompleted: {
        webvfx.registerImageType(video.imageName, video.imageType);
    }

    Connections {
        target: webvfx
        function onRenderRequested(time) {
    	    video.source = webvfx.getImageUrl(video.imageName);
        }
    }

    cache: false
}
