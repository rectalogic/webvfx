import bpy
import math
import numbers
import mathutils
import json

'''
Open a text window in Blender and load this file, then Run Script.
This will add a tool panel (QML Camera Patg) to the Tool Shelf
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
'''

def convertCameraFOV(context, camera):
    '''Blender uses horizontal fov, convert to vertical for Qt3D'''
    render = context.scene.render
    viewportWidth = render.resolution_x * render.pixel_aspect_x
    viewportHeight = render.resolution_y * render.pixel_aspect_y
    return (2 * math.atan(math.tan(camera.data.angle / 2) /
                          (viewportWidth / viewportHeight)))

def getUpVector(matrix):
    # up vector can just be read out of the matrix (y axis)
    return (matrix[1][0], matrix[1][1], matrix[1][2])

def dumpText(context, title, msg):
    text = bpy.data.texts.new(title)
    text.from_string(msg)
    # If an editor is open, switch it to our text
    for area in context.screen.areas:
        if area.type == "TEXT_EDITOR":
            area.active_space.text = text
            break

def reportError(op, msg):
    op.report({'ERROR'}, msg)

class GenerateCameraQml(bpy.types.Operator):
    '''Generate QML Qt3D camera declaration.'''
    bl_idname = "view3d.generate_camera_qml"
    bl_label = "Dump QML Camera"
    bl_description = "Generate QML Qt3D markup for the active camera"

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

        fov = math.degrees(convertCameraFOV(context, camera))

        nearPlane = camera.data.clip_start
        farPlane = camera.data.clip_end

        return ("Viewport {\n"
                "    width: %d\n"
                "    height: %d\n"
                "    camera: Camera {\n"
                "        nearPlane: %f\n"
                "        farPlane: %f\n"
                "        fieldOfView: %f\n"
                "        upVector: Qt.vector3d%s\n"
                "        center: Qt.vector3d%s\n"
                "        eye: Qt.vector3d%s\n"
                "    }\n"
                "}\n"
                "// Rotate models 90 around X axis to convert from Blender coordinate space\n"
                "pretransform: [\n"
                "    Rotation3D {\n"
                "        angle: 90\n"
                "        axis: Qt.vector3d(1, 0, 0)\n"
                "    }\n"
                "]\n" % (render.resolution_x, render.resolution_y,
                         nearPlane, farPlane, fov, up, look, eye))

    def execute(self, context):
        dumpText(context, 'QML Camera', self.generateCameraQml(context))
        return {'FINISHED'}


class SetRenderResolution(bpy.types.Operator):
    '''Set render resolution.'''
    bl_idname = "render.set_resolution"
    bl_label = "Set Render Resolution"
    bl_description = "Set render resolution, which sets camera aspect ratio."

    x = bpy.props.IntProperty()
    y = bpy.props.IntProperty()

    def execute(self, context):
        render = context.scene.render
        render.resolution_x = self.x
        render.pixel_aspect_x = 1
        render.resolution_y = self.y
        render.pixel_aspect_y = 1
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
        up = mathutils.Vector(getUpVector(region_3d.view_matrix.copy().invert()))

        # Get transformed face vertices
        vertices = []
        for v in face.vertices:
            vertices.append(mesh.vertices[v].co * obj.matrix_world)

        # Check the first two edges in the face, pick the one that is
        # parallel to the view up vector.
        # This will be the vertical vector we want to fit viewport height to.
        # Distance between the vertices of that edge is the face height.
        # Use half face height.
        e1 = vertices[1] - vertices[0]
        e2 = vertices[2] - vertices[1]
        epsilon = 0.0005
        if up.angle(e1, 0) <= epsilon:
            height = e1.length / 2
        elif up.angle(e2, 0) <= epsilon:
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
    bl_description = "Generate JSON data for camera animation keyframes"

    CoordNames = ['X', 'Y', 'Z']
    CurveNames = { 'location': 'location', 'rotation_euler': 'rotation'}

    @classmethod
    def poll(cls, context):
        return context.active_object and context.active_object.type == 'CAMERA'

    def generateCameraAnimation(self, action):
        fcurves = action.fcurves
        frame_range = list(action.frame_range)

        animation = {'range': frame_range}
        for f in fcurves:
            name = (self.CurveNames[f.data_path] +
                    self.CoordNames[f.array_index])
            if len(f.keyframe_points) == 1:
                animation[name] = f.keyframe_points[0].co[1]
            else:
                segments = []
                for i in range(len(f.keyframe_points) - 1):
                    k = f.keyframe_points[i]
                    nextk = f.keyframe_points[i+1]
                    segments.append({'range': [k.co[0], nextk.co[0]],
                                     'bezierPoints': [list(k.co),
                                                      list(k.handle_right),
                                                      list(nextk.handle_left),
                                                      list(nextk.co)]})
                animation[name] = segments

        return json.dumps(animation, sort_keys=True, indent=4)

    def execute(self, context):
        action = context.active_object.animation_data.action
        dumpText(context, 'QML Camera Animation', self.generateCameraAnimation(action))
        return {'FINISHED'}


class OBJECT_PT_camera_face_align(bpy.types.Panel):
    '''This functionality can be accessed via the "Tools" panel in 3D View ([T] key).'''
    bl_label = "QML Camera Path"
    bl_space_type = "VIEW_3D"
    bl_region_type = "TOOLS"

    def draw(self, context):
        layout = self.layout

        col = layout.column()
        col.label(text="Face:")
        op_align = col.operator("view3d.viewnumpad", text="1. Align to Face")
        op_align.type = 'TOP'
        op_align.align_active = True
        col.operator("view3d.view_selected", text="2. Center on Face")
        col.operator("view3d.fit_view_to_face", text="3. Fit View to Face")

        col.label(text="Camera:")
        col.operator("view3d.camera_to_view", text="Camera to View")
        col.operator("view3d.generate_camera_qml", text="Generate Camera QML")

        col.label(text="Aspect Ratio:")
        row = col.row()
        op_4_3 = row.operator("render.set_resolution", text="4:3")
        op_4_3.x = 576
        op_4_3.y = 432
        op_16_9 = row.operator("render.set_resolution", text="16:9")
        op_16_9.x = 768
        op_16_9.y = 432

        col.label(text="Camera Keyframes:")
        row = col.row()
        row.operator("anim.insert_camera_keyframe", text="Insert")
        row.operator("anim.remove_camera_keyframe", text="Remove")
        col.operator("anim.generate_camera_animation_json", text="Generate Animation JSON")

# Utility for finding region_3d in console
# def r3d():
#     for area in bpy.context.screen.areas:
#         if area.type == "VIEW_3D":
#             return area.active_space.region_3d
