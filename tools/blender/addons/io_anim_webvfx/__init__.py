# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

bl_info = {
    "name": "WebVfx Object Animation JSON Format",
    "description": "Import/export object animation data in JSON for use with WebVfx",
    "author": "Andrew Wason <rectalogic@rectalogic.com>",
    "version": (1, 0),
    "blender": (2, 5, 8),
    "api": 38205,
    "location": "File > Import-Export",
    "warning": '', # used for warning icon and text in addons panel
    "wiki_url": '',
    "tracker_url": '',
    "category": "Import-Export"
}

# To support reload properly, try to access a package var, if it's there, reload everything
if "bpy" in locals():
    import imp
    if "import_webvfx" in locals():
        imp.reload(import_webvfx)
    if "export_webvfx" in locals():
        imp.reload(export_webvfx)

import bpy
from bpy.props import BoolProperty, StringProperty
from bpy_extras.io_utils import ImportHelper, ExportHelper


KeyframeGroup = "LocRot"
CoordNames = ['X', 'Y', 'Z']
CurveNames = { 'location': 'location', 'rotation_euler': 'rotation'}


class ImportWebVfx(bpy.types.Operator, ImportHelper):
    '''Load WebVfx animation JSON file, replacing animation of the selected object'''
    bl_idname = "import_anim.webvfx"
    bl_label = "Import WebVfx JSON"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = '.js'
    filter_glob = StringProperty(default="*.js", options={'HIDDEN'})

    @classmethod
    def poll(cls, context):
        return context.object

    def execute(self, context):
        from . import import_webvfx
        return import_webvfx.load(self, context, **self.as_keywords(ignore=("filter_glob",)))


class ExportWebVfx(bpy.types.Operator, ExportHelper):
    '''Save WebVfx animation JSON file for the selected object(s)'''
    bl_idname = "export_anim.webvfx"
    bl_label = "Export WebVfx JSON"

    filename_ext = '.js'
    filter_glob = StringProperty(default="*.js", options={'HIDDEN'})

    option_compact = BoolProperty(name="Compact", description="Compact JSON", default=True)
    option_varname = StringProperty(name="var Name", description="JavaScript variable name")

    @classmethod
    def poll(cls, context):
        return context.object and context.object.animation_data

    def execute(self, context):
        from . import export_webvfx
        return export_webvfx.save(self, context, **self.as_keywords(ignore=("check_existing", "filter_glob")))

    def draw(self, context):
        layout = self.layout
        layout.prop(self.properties, "option_compact")
        layout.prop(self.properties, "option_varname")

def menu_func_import(self, context):
    self.layout.operator(ImportWebVfx.bl_idname, text="WebVfx Animation JSON (.js)")


def menu_func_export(self, context):
    self.layout.operator(ExportWebVfx.bl_idname, text="WebVfx Animation JSON (.js)")

def register():
    bpy.utils.register_module(__name__)

    bpy.types.INFO_MT_file_import.append(menu_func_import)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_module(__name__)

    bpy.types.INFO_MT_file_import.remove(menu_func_import)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)

def reportError(op, msg):
    op.report({'ERROR'}, msg)

if __name__ == "__main__":
    register()
