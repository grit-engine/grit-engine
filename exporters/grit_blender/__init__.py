# TODO
# 'create class at cursor'
# 'instantiate class at cursor'
# move object stuff to side panel
# 


bl_info = {
    "name": "Grit Exporter",
    "description": "Exporter for Grit Game Engine",
    "author": "Dave Cunningham",
    "version": (1, 0),
    "blender": (2, 5, 8),
    "api": 31236,
    "location": "File > Import-Export > Grit",
    "warning": "",
    "category": "Import-Export"
}

if "bpy" in locals():
    import imp
    print("Grit exporter: reload")
    #imp.reload(export_mesh_xml)
else:
    print("Grit exporter: load for the first time")
    #from export_mesh_xml import export_obj_to_mesh_xml
    #from export_mesh_xml import *
    #from . import export_mesh_xml

import bpy

from bpy.props import *


from mathutils import Quaternion


# {{{ additional object metadata


bpy.types.Scene.grit_selected_promotion = bpy.props.EnumProperty(name='Grit Object Promotion', default='NONE', items=[('NONE','NONE','Not promoted'),('OBJECT','OBJECT','Grit Object'),('CLASS','CLASS','Grit Class'),('MESH','MESH','Grit .mesh'),('GCOL','GCOL','Grit col')])

bpy.types.Object.grit_promotion = bpy.props.EnumProperty(name='Promotion', default='NONE', items=[('NONE','NONE','Not promoted'),('OBJECT','OBJECT','Grit Object'),('CLASS','CLASS','Grit Class'),('MESH','MESH','Grit .mesh'),('GCOL','GCOL','Grit col')])
bpy.types.Object.grit_object_class_name = bpy.props.StringProperty(name='Class', default="")

# piles

bpy.types.Object.grit_class_rendering_distance = bpy.props.FloatProperty(name='Rendering Distance', default=100)
bpy.types.Object.grit_class_cast_shadows = bpy.props.BoolProperty(name='Casts Shadows?', default=False)
bpy.types.Object.grit_class_place_z_off = bpy.props.FloatProperty(name='Placement Z Offset', default=0.0)
bpy.types.Object.grit_class_place_rnd_rot = bpy.props.BoolProperty(name='Placement Random Z Rotate?', default=False)
#  lights
# health / damage threshold
# colour spec
# explodes, explodeRadius, explodeDeactivate?


class PromoteSelected(bpy.types.Operator):
    '''Make the blender object(s) export to Grit in a particular way'''
    bl_idname = "grit.promote"
    bl_label = "Promote Selected"

    @classmethod
    def poll(cls, context):
        return any(o.grit_promotion != context.scene.grit_selected_promotion for o in context.selected_objects)

    def execute(self, context):
        for o in context.selected_objects:
            o.grit_promotion = context.scene.grit_selected_promotion
        return {'FINISHED'}

#}}}




def export_selected_to_mesh_xml(filepath, scene, obj):
    scene = bpy.context.scene
    for o in bpy.context.selected_objects:
        if o.type == 'MESH':
            export_mesh_xml.export_obj(filepath, scene, o)



             

bpy.types.Scene.grit_mesh_exporter = StringProperty(name=".mesh tool", description="Location of OgreXMLConverter executable", maxlen= 1024, default= "//", subtype="FILE_PATH")
bpy.types.Scene.grit_gcol_exporter = StringProperty(name=".gcol tool", description="Location of grit_col_conv executable", maxlen= 1024, default= "//", subtype="FILE_PATH")

bpy.types.Scene.grit_map_file = StringProperty(name="Grit map path", description="Path of object placements lua file", maxlen= 1024, default= "map.lua")
bpy.types.Scene.grit_map_export = BoolProperty(name="Export map", description="Whether or not to emit an object placements lua file", default=True)
bpy.types.Scene.grit_classes_file = StringProperty(name="Grit classes path", description="Path of class definitions lua file", maxlen=1024, default= "classes.lua")
bpy.types.Scene.grit_classes_export = BoolProperty(name="Export classes", description="Whether or not to emit a class definitions lua file", default=True)
bpy.types.Scene.grit_meshes_export = BoolProperty(name="Export .mesh", description="Whether or not to generate Grit .mesh files", default=True)
bpy.types.Scene.grit_meshes_convert = BoolProperty(name="Convert mesh.xml -> mesh", description="Whether or not to run the external tool", default=True)
bpy.types.Scene.grit_gcols_export = BoolProperty(name="Export .gcol", description="Whether or not to generate Grit .gcol files", default=True)
bpy.types.Scene.grit_gcols_convert = BoolProperty(name="Convert tcol -> gcol", description="Whether or not to run the external tool", default=True)

class SceneSummaryPanel(bpy.types.Panel): # {{{
    bl_idname = "SCENE_PT_grit_summary"
    bl_label = "Grit Exportables Summary"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    def draw(self, context):
        box = self.layout.box()
        func1 = lambda prom, objs: str(len([o for o in objs if o.grit_promotion == prom]))
        func = lambda prom: func1(prom, context.selected_objects) + " of " + func1(prom, context.scene.objects)
        summary = box.row(align=True)
        summary.label("selected objects")
        summary.label(func('OBJECT'))
        summary = box.row(align=True)
        summary.label("selected classes")
        summary.label(func('CLASS'))
        summary = box.row(align=True)
        summary.label("selected meshes")
        summary.label(func('MESH'))
        summary = box.row(align=True)
        summary.label("selected gcols")
        summary.label(func('GCOL'))
        summary = box.row(align=True)
        summary.label("selected unpromoted")
        summary.label(func('NONE'))
#}}}

def to_lua (v):
    if v == True: return "true"
    if v == False: return "false"
    if type(v) == type("a string"): return repr(v)
    return str(v)


def export_objects (scene, objs):
    errors = []
    def append_error (msg):
        nonlocal errors
        if num_errors == 5:
            error_msg = (error_msg or "") + "...\n"
        elif num_errors < 5:
            error_msg = (error_msg or "") + msg + "\n"
        num_errors += 1

    if scene.grit_map_export:
        f = open(scene.grit_map_file, "w")
        f.write("-- Lua file generated by Blender map export script.\n")
        f.write("-- WARNING: If you modify this file, your changes will be lost if it is subsequently re-exported from blender\n\n")

        for obj in objs:
            # should we do something for this blender object?
            if obj.grit_promotion == 'OBJECT':
                if  obj.grit_object_class_name == "":
                    errors.append("Object without class: \""+obj.name+"\"")
                else:
                    pos = obj.location
                    rot = obj.rotation_euler.to_quaternion()
                    rot_str = rot == Quaternion((1,0,0,0)) and "" or "rot=quat("+str(rot.w)+", "+str(rot.x)+", "+str(rot.y)+", "+str(rot.z)+"), "
                    class_name = obj.grit_object_class_name
                    f.write("object \""+class_name+"\" ("+str(pos.x)+", "+str(pos.y)+", "+str(pos.z)+") { "+rot_str+"name=\""+obj.name+"\" }\n")

        f.close()

    if scene.grit_classes_export:
        f = open(scene.grit_classes_file, "w")
        f.write("-- Lua file generated by Blender class export script.\n")
        f.write("-- WARNING: If you modify this file, your changes will be lost if it is subsequently re-exported from blender\n\n")

        for obj in objs:
            # should we do something for this blender object?
            if obj.grit_promotion == 'CLASS':
                class_name = obj.name
                if class_name[0] == '!': class_name = class_name[1:]

                meshes = []
                cols = []

                for c in obj.children:
                    if c.grit_promotion == 'MESH':
                        meshes.append(c)
                    if c.grit_promotion == 'GCOL':
                        cols.append(c)
                if len(meshes) == 0:
                    errors.append("Class does not have a mesh: \""+obj.name+"\"\n")
                    continue
                if len(meshes) > 1:
                    errors.append("Class has "+len(meshes)+" meshes (should have 1): \""+obj.name+"\"\n")
                    continue

                attributes = []
                attributes.append(("renderingDistance", obj.grit_class_rendering_distance))
                attributes.append(("castShadows", obj.grit_class_cast_shadows))

                if class_name + ".mesh" != meshes[0].name: attributes.append("gfxMesh", meshes[0].name)
                parent_class_name = ""
                if len(cols) == 0:
                    parent_class_name = "BaseClass"
                    interior = " "
                else:
                    parent_class_name = "ColClass"
                    if len(cols) > 1:
                        errors.append("Class has "+len(meshes)+" cols (should have 0 or 1): \""+obj.name+"\"")
                        continue
                    if class_name + ".gcol" != cols[0].name: attributes.append("colMesh", cols[0].name)

                interior = ""
                if len(attributes) == 0:
                    interior = " "
                else:
                    interior = "\n    " + ";\n    ".join(map(lambda x: x[0] + " = " + to_lua(x[1]), attributes)) + ";\n"

                f.write("class \""+class_name+"\" ("+parent_class_name+") {"+interior+"}\n\n")

        f.close()

    # clip errors at a max to avoid filling the screen with crap
    max_errors = 20
    if len(errors) > max_errors: errors = errors[:max_errors] + ["..."]
    if len(errors) > 0: return "\n".join(errors)

class ScenePanel(bpy.types.Panel): #{{{
    bl_idname = "SCENE_PT_grit"
    bl_label = "Grit Export Settings"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "scene"

    #@classmethod
    #def poll(cls, context):
    #    return len(context.selected_objects) == 1

    def draw(self, context):
        box = self.layout
        box.prop(context.scene, "grit_mesh_exporter", icon="FILE_SCRIPT")
        box.prop(context.scene, "grit_gcol_exporter", icon="FILE_SCRIPT")
        row = box.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_map_export")
        row.prop(context.scene, "grit_map_file", text="", expand=True)
        row = box.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_classes_export")
        row.prop(context.scene, "grit_classes_file", text="", expand=True)
        row = box.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_meshes_export")
        row.prop(context.scene, "grit_meshes_convert", text="Convert", expand=True)
        row = box.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_gcols_export")
        row.prop(context.scene, "grit_meshes_convert", text="Convert", expand=True)
        row = box.row()
        row.operator("grit.export_selected", icon="SCRIPT")
        row.operator("grit.export_all", icon="SCRIPT")
#}}}


class ExportScene(bpy.types.Operator):
    '''Export scene to grit files'''
    bl_idname = "grit.export_all"
    bl_label = "Export Whole Scene"

    @classmethod
    def poll(cls, context):
        return any(x.grit_promotion != 'NONE' for x in context.scene.objects)

    def execute(self, context):
        scene = bpy.context.scene
        objs = scene.objects
        err = export_objects(scene, objs)
        print(err)
        print(type(err))
        if err != None: self.report({'ERROR_INVALID_INPUT'}, err)
        return {'FINISHED'}


class ExportSelected(bpy.types.Operator):
    '''Export selected objects to grit files'''
    bl_idname = "grit.export_selected"
    bl_label = "Export Selected"

    @classmethod
    def poll(cls, context):
        return any(x.grit_promotion != 'NONE' for x in context.selected_objects)

    def execute(self, context):
        scene = bpy.context.scene
        objs = bpy.context.selected_objects
        err = export_objects(scene, objs)
        print(err)
        print(type(err))
        if err != None: self.report({'ERROR_INVALID_INPUT'}, err)
        return {'FINISHED'}


class ApplyClassToAllSelected(bpy.types.Operator):
    '''Make all selected objects' Grit classes like the active object's'''
    bl_idname = "grit.apply_class"
    bl_label = "Apply to selection"

    @classmethod
    def poll(cls, context):
        if context.active_object == None: return False
        if len(context.selected_objects) == 0: return False
        if context.active_object.grit_promotion != "OBJECT": return False
        if any(x.grit_promotion != 'OBJECT' for x in context.selected_objects): return False
        if all(x.grit_object_class_name == context.active_object.grit_object_class_name for x in context.selected_objects): return False
        return True

    def execute(self, context):
        for o in context.selected_objects:
            if o.grit_promotion == "OBJECT":
                o.grit_object_class_name = context.active_object.grit_object_class_name
        return {'FINISHED'}


class ObjectPanel(bpy.types.Panel):
    bl_idname = "OBJECT_PT_grit_object_properties"
    bl_label = "Grit Object Properties"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        obj = context.active_object
        row = self.layout
        row.prop(obj, "grit_object_class_name")
        if obj == None or obj.grit_promotion != "OBJECT":
            row.enabled = False
        row.operator("grit.apply_class")
        

class SelectedPanel(bpy.types.Panel):
    bl_idname = "OBJECT_PT_grit_selected"
    bl_label = "Grit Selection Tools"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        sel_objs = context.selected_objects

        row = self.layout.row()
        row.alignment = "EXPAND"
        row.operator("grit.promote", text="Promote selected to:")
        row.prop(context.scene, "grit_selected_promotion", text="")



def register():
    bpy.utils.register_module(__name__)

def unregister():
    bpy.utils.unregister_module(__name__)
