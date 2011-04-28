# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import bpy
import bpy.props
import math
import numbers
import mathutils
import json
from io_utils import ImportHelper


bl_info = {
    "name": "WebVfx Camera Animation",
    "description": "Set of tools to help create camera animation data for WebVfx.",
    "author": "Andrew Wason <rectalogic@rectalogic.com>",
    "version": (1, 0),
    "blender": (2, 5, 7),
    "api": 36007,
    "location": "View3D > ToolShelf > WebVfx Camera Animation",
    "warning": '', # used for warning icon and text in addons panel
    "wiki_url": '',
    "tracker_url": '',
    "category": "Animation"
}

'''
Open a text window in Blender and load this file, then Run Script.
This will add a tool panel (WebVfx Camera Animation) to the Tool Shelf
in the 3D view (type T to show the shelf).

Select an object whose face you want to align to the camera,
hit TAB to switch to mesh edit mode, select the Face selector tool
and select the face you want to align.

Click the Align to Face, then Center on Face, then Fit View to Face
buttons. Then Camera to View.
This should be keyframed as the first or last keyframe - select
the camera object and click Insert under Camera Keyframes.

Now you reposition the timeline and add additional camera keyframes.
Change the Timeline view to FCurve view to edit the animation curves.

When a complete camera animation timeline has been constructed,
click Generate Animation JSON. This will dump the JSON
to the text view.

Click Generate Camera QML to generate the camera fieldOfView
and other parameters. This also dumps the QML for the current camera position.

The Aspect Ratio buttons can be used to change the camera viewport
aspect ratio.

Enabling the Measure Panel addon (via User Preferences) is useful
to determine the aspect ratio of a quad that will render text.
'''

def convertCameraFOV(context, camera):
    '''Blender uses horizontal fov, convert to vertical for QtQuick3D'''
    render = context.scene.render
    viewportWidth = render.resolution_x * render.pixel_aspect_x
    viewportHeight = render.resolution_y * render.pixel_aspect_y
    return (2 * math.atan(math.tan(camera.data.angle / 2) /
                          (viewportWidth / viewportHeight)))

def getUpVector(matrix):
    # up vector can just be read out of the matrix (y axis)
    return (matrix[1][0], matrix[1][1], matrix[1][2])

def dumpText(operator, context, title, msg):
    text = bpy.data.texts.new(title)
    text.from_string(msg)
    # If an editor is open, switch it to our text
    for area in context.screen.areas:
        if area.type == "TEXT_EDITOR":
            area.active_space.text = text
            break
    operator.report({'INFO'}, "Output in %s" % text.name)

def reportError(op, msg):
    op.report({'ERROR'}, msg)

class GenerateCameraQml(bpy.types.Operator):
    '''Generate QML QtQuick3D camera declaration.'''
    bl_idname = "view3d.generate_camera_qml"
    bl_label = "Dump QML Camera"
    bl_description = "Generate QML QtQuick3D markup for the active camera"

    def generateCameraQml(self, context):
        scene = context.scene
        render = scene.render
        camera = scene.camera
        matrix = camera.matrix_world

        # eye position can just be read out of the matrix
        w = matrix[3][3]
        eye = (matrix[3][0] / w, matrix[3][1] / w, matrix[3][2] / w)
        # get the dir vector (camera looking down z)
        direction = (matrix[2][0], matrix[2][1], matrix[2][2])
        # look is just the eye position - the direction
        look = (eye[0] - direction[0], eye[1] - direction[1], eye[2] - direction[2])
        up = getUpVector(matrix)

        # Blender fov is horizontal, QtQuick3D is vertical.
        # Precompute part factor to convert at runtime based on viewport size.
        fovFactor = math.tan(camera.data.angle / 2)

        nearPlane = camera.data.clip_start
        farPlane = camera.data.clip_end

        return ("Viewport {\n"
                "    camera: Camera {\n"
                "        nearPlane: %f\n"
                "        farPlane: %f\n"
                "        fieldOfView: (2 * Math.atan(%f / (width / height))) * 180 / Math.PI\n"
                "\n"
                "        upVector: Qt.vector3d%s\n"
                "        center: Qt.vector3d%s\n"
                "        eye: Qt.vector3d%s\n"
                "    }\n"
                "}\n" % (nearPlane, farPlane, fovFactor, up, look, eye))

    def execute(self, context):
        dumpText(self, context, 'QML Camera', self.generateCameraQml(context))
        return {'FINISHED'}


class SetAspectRatio(bpy.types.Operator):
    '''Set camera aspect ratio.'''
    bl_idname = "render.set_aspect_ratio"
    bl_label = "Set Aspect Ratio"
    bl_description = "Set aspect ratio."

    ratio = bpy.props.EnumProperty(items=(("16:9", "16:9", "16:9"),
                                          ("4:3", "4:3", "4:3")),
                                   name="Aspect Ratio",
                                   description="Aspect Ratio",
                                   default="4:3")

    def execute(self, context):
        render = context.scene.render
        render.pixel_aspect_x = 1
        render.pixel_aspect_y = 1
        if self.ratio == "16:9":
            render.resolution_x = 768
            render.resolution_y = 432
        elif self.ratio == "4:3":
            render.resolution_x = 576
            render.resolution_y = 432
        return {'FINISHED'}


class FitViewToFace(bpy.types.Operator):
    '''Move view so face height will fill the cameras viewport.
    View should already be aligned and centered on a face.'''
    bl_idname = "view3d.fit_view_to_face"
    bl_label = "Fit View to Face"
    bl_description = "Move view so selected face will fill camera viewport"

    @classmethod
    def poll(cls, context):
        return context.area.type == 'VIEW_3D' and context.active_object and context.active_object.type == 'MESH'

    def fitViewToFace(self, context):
        obj = context.active_object
        mesh = obj.data
        face = mesh.faces[obj.data.faces.active]

        region_3d = context.space_data.region_3d
        center = face.center * obj.matrix_world

        # Make sure view location is very close to face center
        if (center - region_3d.view_location).length > 3e-07:
            reportError(self, "View not aligned on active face center")
            return

        # Get views up vector
        up = mathutils.Vector(getUpVector(region_3d.view_matrix.copy().inverted()))

        # Get transformed face vertices
        vertices = []
        for v in face.vertices:
            vertices.append(mesh.vertices[v].co * obj.matrix_world)

        # Check the first two edges in the face, pick the one that is
        # parallel to the view up vector (angle is 0 or 180).
        # This will be the vertical vector we want to fit viewport height to.
        # Distance between the vertices of that edge is the face height.
        # Use half face height.
        e1 = vertices[1] - vertices[0]
        e2 = vertices[2] - vertices[1]
        a1 = abs(up.angle(e1, 0))
        a2 = abs(up.angle(e2, 0))
        epsilon = 0.0005
        if a1 <= epsilon or (abs(a1 - math.pi) <= epsilon):
            height = e1.length / 2
        elif a2 <= epsilon or (abs(a2 - math.pi) <= epsilon):
            height = e2.length / 2
        else:
            reportError(self, "Could not find vertical edge in face")
            return

        # Angle from eye, halve for triangle.
        fov = convertCameraFOV(context, context.scene.camera) / 2

        # Use law of sines to get view_distance
        # http://en.wikipedia.org/wiki/Law_of_sines

        # pi/2 rad is 90 (the right angle), pi rad is 180
        # So the remaining angle is 1 - pi/2 - fov
        context.space_data.region_3d.view_distance = \
            math.sin(math.pi - math.pi/2 - fov) * (height / math.sin(fov))

    def execute(self, context):
        # Toggle out of editmode to update mesh faces.active
        bpy.ops.object.editmode_toggle()
        bpy.ops.object.editmode_toggle()

        self.fitViewToFace(context)
        return {'FINISHED'}


class RotateView90(bpy.types.Operator):
    '''Rotate view by 90 degrees'''
    bl_idname = "view3d.rotate_view_90"
    bl_label = "Rotate View 90"
    bl_description = "Rotate view by 90 degrees"

    @classmethod
    def poll(cls, context):
        return context.area.type == 'VIEW_3D'

    def execute(self, context):
        region_3d = context.space_data.region_3d
        view_matrix = region_3d.view_matrix.copy()
        rot90 = mathutils.Matrix.Rotation(math.radians(90), 3, 'Z')
        region_3d.view_rotation = (view_matrix.to_3x3().inverted() * rot90).inverted().to_quaternion()

        return {'FINISHED'}


class InsertCameraKeyframe(bpy.types.Operator):
    bl_idname = "anim.insert_camera_keyframe"
    bl_label = "Insert Camera Keyframe"
    bl_description = "Insert a new keyframe, or update current keyframe for the active camera"

    def execute(self, context):
        # Without index, inserts all keys of given type.
        # Without frame, uses current timeline frame.
        camera = context.scene.camera
        camera.keyframe_insert('location', group='LocRot')
        camera.keyframe_insert('rotation_euler', group='LocRot')
        return {'FINISHED'}


class RemoveCameraKeyframe(bpy.types.Operator):
    bl_idname = "anim.remove_camera_keyframe"
    bl_label = "Remove Camera Keyframe"
    bl_description = "Remove current keyframe for the active camera"

    def execute(self, context):
        camera = context.scene.camera
        camera.keyframe_delete('location', group='LocRot')
        camera.keyframe_delete('rotation_euler', group='LocRot')
        return {'FINISHED'}


class GenerateCameraAnimationJson(bpy.types.Operator):
    bl_idname = "anim.generate_camera_animation_json"
    bl_label = "Generate Camera Animation JSON"
    bl_description = "Generate JSON data for active camera animation keyframes"

    CoordNames = ['X', 'Y', 'Z']
    CurveNames = { 'location': 'location', 'rotation_euler': 'rotation'}

    # Adjust control points so the total length of the "handles"
    # is not more than the horizontal distance between the keyframe points.
    def correctControlPoints(self, points):
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

    def generateCameraAnimation(self, camera):
        action = camera.animation_data.action
        fcurves = action.fcurves
        frame_range = list(action.frame_range)

        animation = {'horizontalFOV': camera.data.angle, 'range': frame_range}
        for f in fcurves:
            name = (self.CurveNames[f.data_path] +
                    self.CoordNames[f.array_index])
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
                    self.correctControlPoints(points)
                    segments.append({'range': [k.co[0], nextk.co[0]],
                                     'bezierPoints': points})
                animation[name] = segments

        return json.dumps(animation, sort_keys=True, indent=4,
                          separators=(',',': '))

    def execute(self, context):
        dumpText(self, context, 'JSON Camera Animation',
                 self.generateCameraAnimation(context.scene.camera))
        return {'FINISHED'}


class ImportCameraAnimationJson(bpy.types.Operator, ImportHelper):
    bl_idname = "anim.import_camera_animation_json"
    bl_label = "Import Camera Animation JSON"
    bl_description = "Import JSON data for active camera animation keyframes"

    filename_ext = "*.json"
    filter_glob = bpy.props.StringProperty(default="*.json", options={'HIDDEN'})

    def importCameraAnimation(self, camera, filepath):
        with open(filepath, "r") as file:
            animation = json.load(file)

        camera.animation_data_clear()
        camera.animation_data_create()

        action = bpy.data.actions.new("CameraAction")
        groupName = "LocRot"
        action.groups.new(groupName)

        camera.data.angle = animation['horizontalFOV']
        animRange = animation['range']

        for curve in GenerateCameraAnimationJson.CurveNames:
            for coord in range(3):
                fcurve = action.fcurves.new(curve, coord, groupName)
                curveData = animation[GenerateCameraAnimationJson.CurveNames[curve] +
                                      GenerateCameraAnimationJson.CoordNames[coord]]
                # Constant value
                if isinstance(curveData, numbers.Real):
                    fcurve.keyframe_points.insert(animRange[0], curveData)
                # Array of segment control point data
                else:
                    for segment in curveData:
                        points = segment['bezierPoints']
                        keyframe = fcurve.keyframe_points.insert(points[0][0],
                                                                 points[0][1])
                        keyframe.handle_right = points[1]
                        keyframe = fcurve.keyframe_points.insert(points[3][0],
                                                                 points[3][1])
                        keyframe.handle_left = points[2]

        camera.animation_data.action = action

    def execute(self, context):
        # Display filechooser for *.json files
        self.importCameraAnimation(context.scene.camera,
                                   **self.as_keywords(ignore=("filter_glob",)))
        return {'FINISHED'}


class OBJECT_PT_camera_face_align(bpy.types.Panel):
    '''This functionality can be accessed via the "Tools" panel in 3D View ([T] key).'''
    bl_label = "WebVfx Camera Animation"
    bl_space_type = "VIEW_3D"
    bl_region_type = "TOOLS"

    def draw(self, context):
        layout = self.layout

        col = layout.column(align=True)

        col.label(text="Face:")
        op_align = col.operator("view3d.viewnumpad", text="1. Align to Face")
        op_align.type = 'TOP'
        op_align.align_active = True
        col.operator("view3d.view_selected", text="2. Center on Face")
        col.operator("view3d.rotate_view_90", text="2.5 Rotate View 90")
        col.operator("view3d.fit_view_to_face", text="3. Fit View to Face")

        col.label(text="Camera:")
        col.operator("view3d.camera_to_view", text="Camera to View")
        col.operator("view3d.generate_camera_qml", text="Generate Camera QML")

        col.label(text="Aspect Ratio:")
        row = col.row()
        op_4_3 = row.operator("render.set_aspect_ratio", text="4:3").ratio="4:3"
        op_16_9 = row.operator("render.set_aspect_ratio", text="16:9").ratio="16:9"

        col.label(text="Horizontal FOV:")
        col.prop(context.scene.camera.data, "angle")

        col.label(text="Camera Keyframes:")
        row = col.row()
        row.operator("anim.insert_camera_keyframe", text="Insert")
        row.operator("anim.remove_camera_keyframe", text="Remove")
        col.operator("anim.generate_camera_animation_json", text="Generate Animation JSON")
        col.operator("anim.import_camera_animation_json", text="Import Animation JSON")

# Utility for finding region_3d in console
# def r3d():
#     for area in bpy.context.screen.areas:
#         if area.type == "VIEW_3D":
#             return area.active_space.region_3d


def register():
    bpy.utils.register_module(__name__)
    pass

def unregister():
    bpy.utils.unregister_module(__name__)
    pass

if __name__ == "__main__":
    register()
