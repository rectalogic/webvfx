// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Qt 4.7
import Qt3D 1.0

// Use this to import models exported from Blender.
// Converts from Blender to Qt3D coordinate system.

Item3D {
    pretransform: [
        Rotation3D {
            angle: 90
            axis: Qt.vector3d(1, 0, 0)
        }
    ]
}
