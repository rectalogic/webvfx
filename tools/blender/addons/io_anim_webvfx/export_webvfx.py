# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import bpy
import math
import json
from io_anim_webvfx import CurveNames, CoordNames

# Adjust control points so the total length of the "handles"
# is not more than the horizontal distance between the keyframe points.
def correctControlPoints(points):
    # Handle deltas
    h1 = [points[0][0] - points[1][0], points[0][1] - points[1][1]]
    h2 = [points[3][0] - points[2][0], points[3][1] - points[2][1]]

    length = points[3][0] - points[0][0]
    len1 = math.fabs(h1[0])
    len2 = math.fabs(h2[0])

    if (len1 + len2) == 0:
        return

    # The handles cross, force apart by proportion of overlap
    if (len1 + len2) > length:
        overlap = length / (len1 + len2)
        points[1][0] = points[0][0] - overlap * h1[0]
        points[1][1] = points[0][1] - overlap * h1[1]
        points[2][0] = points[3][0] - overlap * h2[0]
        points[2][1] = points[3][1] - overlap * h2[1]

def buildAnimation(animObject):
    action = animObject.animation_data.action
    fcurves = action.fcurves
    frame_range = list(action.frame_range)

    animation = {'range': frame_range}
    # Save FOV if object is a Camera
    if animObject.type == 'CAMERA':
        animation['horizontalFOV'] = animObject.data.angle

    for f in fcurves:
        name = (CurveNames[f.data_path] + CoordNames[f.array_index])
        # Single keyframe - use constant value
        if len(f.keyframe_points) == 1:
            animation[name] = f.keyframe_points[0].co[1]
        # Otherwise get the 4 control points - this keyframes point
        # and it's right handle, then the next keyframes left handle
        # and the next keyframes point
        else:
            segments = []
            for i in range(len(f.keyframe_points) - 1):
                k = f.keyframe_points[i]
                nextk = f.keyframe_points[i+1]
                points = [list(k.co),
                          list(k.handle_right),
                          list(nextk.handle_left),
                          list(nextk.co)]
                correctControlPoints(points)
                segments.append({'range': [k.co[0], nextk.co[0]],
                                 'bezierPoints': points})
            animation[name] = segments

    return animation

def save(operator, context, option_compact, option_varname, filepath=""):
    if len(context.selected_objects) > 1:
        animation = []
        for obj in context.selected_objects:
            if obj.animation_data:
                animation.append(buildAnimation(obj))
    else:
        animation = buildAnimation(context.object)

    animationJS = ""
    if option_varname:
        animationJS = "var %s =\n" % option_varname

    if option_compact:
        animationJS += json.dumps(animation, sort_keys=True,
                                  separators=(',',':'))
    else:
        animationJS += json.dumps(animation, sort_keys=True, indent=4,
                                  separators=(',',': '))

    with open(filepath, "w") as file:
        file.write(animationJS)
    return {'FINISHED'}
