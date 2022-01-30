// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import QtQuick

Image {
    id: video
    required property string imageName

    Component.onCompleted: {
        webvfx.registerImageName(video.imageName);
    }

    Connections {
        target: webvfx
        function onRenderRequested(time) {
    	    video.source = webvfx.getImageUrl(video.imageName);
        }
    }

    cache: false
}
