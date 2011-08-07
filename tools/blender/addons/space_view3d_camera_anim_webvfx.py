# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import bpy
import bpy.props
import math
import mathutils


bl_info = {
    "name": "WebVfx Camera Animation",
    "description": "Set of tools to help create camera animation for WebVfx.",
    "author": "Andrew Wason <rectalogic@rectalogic.com>",
    "version": (1, 0),
    "blender": (2, 5, 8),
    "api": 38205,
    "location": "View3D > ToolShelf > WebVfx Camera Animation",
    "warning": '', # used for warning icon and text in addons panel
    "wiki_url": '',
    "tracker_url": '',
    "category": "Animation"
}

'''
Adds a tool panel (WebVfx Camera Animation) to the Tool Shelf
in the 3D view (type T to show the shelf).

Select an object whose face you want to align to the camera,
hit TAB to switch to mesh edit mode, select the Face selector tool
and select the face you want to align.

Click Align, then Center, then (optionally) Rotate 90, then Fit.
Then Camera to View to move the camera to the current view.
This should be keyframed as the first or last keyframe - select
the camera object and click Insert under Keyframes.

Now you reposition the timeline and add additional camera keyframes.
Change the Timeline view to FCurve view to edit the animation curves.

When a complete camera animation timeline has been constructed,
use the export addon to export WebVfx JSON.

Click Dump Camera QML to generate the camera fieldOfView
and other parameters. This also dumps the QML for the current camera position.

The Aspect Ratio buttons can be used to change the camera viewport
aspect ratio. If using the same animation with both 4:3 and 16:9 faces,
make both quads the same width (but different heights) and center them
on the same location. The same animation will then work with both the
4:3 and 16:9 quads - just change the camera aspect ratio for whichever
quad you are viewing.

Enabling the Measure Panel addon (via User Preferences) is useful
to determine the aspect ratio of a quad that will render text.
'''

KeyframeGroup = "LocRot"


class Eye:
    def __init__(self, matrix):
        # up vector can just be read out of the matrix (y axis)
        self.upVector = getUpVector(matrix)
        # eye position can just be read out of the matrix
        w = matrix[3][3]
        eye = (matrix[3][0] / w, matrix[3][1] / w, matrix[3][2] / w)
        self.eyeVector = eye
        # get the direction vector (camera looking down z)
        direction = (matrix[2][0], matrix[2][1], matrix[2][2])
        # lookat is just the eye position - the direction
        self.lookAtVector = (eye[0] - direction[0], eye[1] - direction[1], eye[2] - direction[2])

def getUpVector(matrix):
    # up vector can just be read out of the matrix (y axis)
    return (matrix[1][0], matrix[1][1], matrix[1][2])

def dumpText(operator, context, title, msg):
    text = bpy.data.texts.new(title)
    text.from_string(msg)
    # If an editor is open, switch it to our text
    for area in context.screen.areas:
        if area.type == "TEXT_EDITOR":
            area.spaces.active.text = text
            break
    operator.report({'INFO'}, "Output in %s" % text.name)

def reportError(op, msg):
    op.report({'ERROR'}, msg)

class GenerateCameraQml(bpy.types.Operator):
    '''Generate QtQuick3D QML camera declaration into a text block.'''
    bl_idname = "view3d.generate_camera_qml"
    bl_label = "Dump QML Camera"
    bl_description = "Generate QtQuick3D QML markup for the active camera"

    def generateCameraQml(self, context):
        scene = context.scene
        render = scene.render
        camera = scene.camera
        matrix = camera.matrix_world

        eye = Eye(matrix)

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
                "}\n" % (nearPlane, farPlane, fovFactor, eye.upVector, eye.lookAtVector, eye.eyeVector))

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

    def convertCameraFOV(self, context, camera):
        '''Blender uses horizontal fov, convert to vertical for QtQuick3D'''
        render = context.scene.render
        viewportWidth = render.resolution_x * render.pixel_aspect_x
        viewportHeight = render.resolution_y * render.pixel_aspect_y
        return (2 * math.atan(math.tan(camera.data.angle / 2) /
                              (viewportWidth / viewportHeight)))

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
        fov = self.convertCameraFOV(context, context.scene.camera) / 2

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
        view_matrix = region_3d.view_matrix.inverted().to_3x3()
        rot90 = mathutils.Euler((0, 0, math.pi / 2))
        view_matrix.rotate(rot90)
        region_3d.view_rotation = view_matrix.to_quaternion()
        return {'FINISHED'}


class InsertCameraKeyframe(bpy.types.Operator):
    bl_idname = "anim.insert_camera_keyframe"
    bl_label = "Insert Camera Keyframe"
    bl_description = "Insert a new keyframe, or update current keyframe for the active camera"

    def execute(self, context):
        # Without index, inserts all keys of given type.
        # Without frame, uses current timeline frame.
        camera = context.scene.camera
        camera.keyframe_insert('location', group=KeyframeGroup)
        camera.keyframe_insert('rotation_euler', group=KeyframeGroup)
        return {'FINISHED'}


class RemoveCameraKeyframe(bpy.types.Operator):
    bl_idname = "anim.remove_camera_keyframe"
    bl_label = "Remove Camera Keyframe"
    bl_description = "Remove current keyframe for the active camera"

    def execute(self, context):
        camera = context.scene.camera
        camera.keyframe_delete('location', group=KeyframeGroup)
        camera.keyframe_delete('rotation_euler', group=KeyframeGroup)
        return {'FINISHED'}


class OBJECT_PT_camera_face_align(bpy.types.Panel):
    '''This functionality can be accessed via the "Tools" panel in 3D View ([T] key).'''
    bl_label = "WebVfx Camera Animation"
    bl_space_type = "VIEW_3D"
    bl_region_type = "TOOLS"

    def draw(self, context):
        layout = self.layout

        box = layout.box()
        box.label(text="View to Selected Face")
        box.label(text="Select a quad face.\nThen align, center, rotate and fit the viewport to it.\nThen move the camera to the view.")
        col = box.column(align=True)

        op_align = col.operator("view3d.viewnumpad", text="1. Align")
        op_align.type = 'TOP'
        op_align.align_active = True
        col.operator("view3d.view_selected", text="2. Center")
        col.operator("view3d.rotate_view_90", text="2.5 Rotate 90")
        col.operator("view3d.fit_view_to_face", text="3. Fit Viewport")

        box = layout.box()
        box.label(text="Camera")
        col = box.column(align=True)

        col.operator("view3d.camera_to_view", text="Camera to View")

        col.label(text="Aspect Ratio:")
        row = col.row()
        op_4_3 = row.operator("render.set_aspect_ratio", text="4:3").ratio="4:3"
        op_16_9 = row.operator("render.set_aspect_ratio", text="16:9").ratio="16:9"

        col.label(text="Horizontal FOV:")
        col.prop(context.scene.camera.data, "angle")

        col.label(text="Keyframes:")
        row = col.row()
        row.operator("anim.insert_camera_keyframe", text="Insert")
        row.operator("anim.remove_camera_keyframe", text="Remove")

        layout.operator("view3d.generate_camera_qml", text="Dump Camera QML")

# Utility for finding region_3d in console
# def r3d():
#     for area in bpy.context.screen.areas:
#         if area.type == "VIEW_3D":
#             return area.spaces.active.region_3d


def register():
    bpy.utils.register_module(__name__)
    pass

def unregister():
    bpy.utils.unregister_module(__name__)
    pass

if __name__ == "__main__":
    register()
