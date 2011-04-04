// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Qt 4.7
import WebVfx 1.0

// Exposes rendered text as a texture image.
// We render inside a transparent rectangle to ensure the width/height
// is honoroed so when we texture we don't get distorted.

Rectangle {
    property alias text: internalText.text
    property alias color: internalText.color
    property alias font: internalText.font
    property alias textureImage: capturedText.pixmap

    // Transparent background
    color: "#00000000"
    // Apply effect that captures all rendering to an image
    effect: Capture { id: capturedText }

    Text {
        id: internalText
        smooth: true
        width: parent.width
        height: parent.height
    }
}
