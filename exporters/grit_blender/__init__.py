# TODO
# 'create class at cursor'


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

import bpy
from bpy.props import *
from mathutils import Quaternion


def strip_leading_exc(x):
    if x[0] == '!':
        return x[1:]


# {{{ export xml utility

def float_eq (x, y):
    # no reason to handle rounding errors at this point
    return x == y

def uv_eq (x, y):
    return float_eq(x[0], y[0]) and float_eq(x[1], y[1])

def export_mesh_xml(mesh, filename):
    num_uv = len(mesh.uv_textures)

    class Empty: pass

    # list of vertexes with their attributes
    vertexes = []

    # table mapping material name to a list of face triples
    faces = { }

    counter = 0

    for fi, f in enumerate(mesh.faces):

        matname = mesh.materials[f.material_index].name

        triangles = [[f.vertices[0], f.vertices[1], f.vertices[2]]] 
        if len(f.vertices) == 4:
            triangles.append([f.vertices[0], f.vertices[2], f.vertices[3]])

        for triangle in triangles:
            face = [0,0,0]
            for fvi, vi in enumerate(triangle):
                v = mesh.vertices[vi]
                vert = Empty()
                vert.pos =  v.co
                vert.normal = v.normal
                if (num_uv > 0):
                    vert.uv = mesh.uv_textures[0].data[fi].uv[fvi]
                else:
                    vert.uv = [0,0]

                # see if we already hvae a vertex that is close enough
                duplicate = False
                for evi, evert in enumerate(vertexes): #existing vertex id
                    if (evert.pos - vert.pos).length < 0.00001 and \
                       (evert.normal - vert.normal).length < 0.00001 and \
                       uv_eq(evert.uv, vert.uv):
                        duplicate = evi
                        break

                if duplicate:
                    face[fvi] = duplicate
                else:
                    vertexes.append(vert)
                    face[fvi] = counter
                    counter += 1
            if not matname in faces.keys():
                # first face we have seen of this material
                faces[matname] = []
            faces[matname].append(face)


    #actually write the file
    file = open(filename, "w")
    file.write("<mesh>\n")
    file.write("    <sharedgeometry>\n")
    file.write("        <vertexbuffer positions=\"true\" normals=\"true\" texture_coord_dimensions_0=\"float2\" texture_coords=\"1\">\n")
    for v in vertexes:
        file.write("            <vertex>\n")
        file.write("                <position x=\""+str(v.pos.x)+"\" y=\""+str(v.pos.y)+"\" z=\""+str(v.pos.z)+"\" />\n")
        file.write("                <normal x=\""+str(v.normal.x)+"\" y=\""+str(v.normal.y)+"\" z=\""+str(v.normal.z)+"\" />\n")
        file.write("                <texcoord u=\""+str(v.uv[0])+"\" v=\""+str(v.uv[1])+"\" />\n")
        file.write("            </vertex>\n")
    file.write("        </vertexbuffer>\n")
    file.write("    </sharedgeometry>\n")
    file.write("    <submeshes>\n")
    for m in faces.keys():
        file.write("        <submesh material=\""+m+"\" usesharedvertices=\"true\" use32bitindexes=\"false\" operationtype=\"triangle_list\">\n")
        file.write("            <faces>\n")
        for f in faces[m]:
            file.write("                <face v1=\""+str(f[0])+"\" v2=\""+str(f[1])+"\" v3=\""+str(f[2])+"\" />\n")
        file.write("            </faces>\n")
        file.write("        </submesh>\n")
    file.write("    </submeshes>\n")
    file.write("</mesh>\n")
    file.close()

def export_mesh_tcol(mesh, filename):

    #actually write the file
    file = open(filename, "w")
    file.write("TCOL1.0\n")
    file.write("\n")
    file.write("attributes {\n")
    file.write("    static;\n")
    file.write("}\n")
    file.write("\n")
    file.write("compound {\n")
    file.write("}\n")
    file.close()

# }}} 


#{{{ Scene stuff             

promotion_enum_items = [
    ('NONE','No Export','Not promoted'),
    ('OBJECT','Object','Grit Object'),
    ('CLASS','Class','Grit Class'),
    ('MESH','.mesh','Grit .mesh'),
    ('GCOL','col','Grit col'),
]

bpy.types.Scene.grit_mesh_exporter = StringProperty(name=".mesh tool", description="Location of OgreXMLConverter executable", maxlen= 1024, default= "//", subtype="FILE_PATH")
bpy.types.Scene.grit_gcol_exporter = StringProperty(name=".gcol tool", description="Location of grit_col_conv executable", maxlen= 1024, default= "//", subtype="FILE_PATH")
bpy.types.Scene.grit_selected_promotion = bpy.props.EnumProperty(name='Grit Object Promotion', default='NONE', items=promotion_enum_items)

bpy.types.Scene.grit_map_file = StringProperty(name="Grit map path", description="Path of object placements lua file", maxlen= 1024, default= "map.lua")
bpy.types.Scene.grit_map_export = BoolProperty(name="Export map", description="Whether or not to emit an object placements lua file", default=True)
bpy.types.Scene.grit_classes_file = StringProperty(name="Grit classes path", description="Path of class definitions lua file", maxlen=1024, default= "classes.lua")
bpy.types.Scene.grit_classes_export = BoolProperty(name="Export classes", description="Whether or not to emit a class definitions lua file", default=True)
bpy.types.Scene.grit_meshes_export = BoolProperty(name="Export .mesh", description="Whether or not to generate Grit .mesh files", default=True)
bpy.types.Scene.grit_meshes_convert = BoolProperty(name="Convert mesh.xml -> mesh", description="Whether or not to run the external tool", default=True)
bpy.types.Scene.grit_gcols_export = BoolProperty(name="Export .gcol", description="Whether or not to generate Grit .gcol files", default=True)
bpy.types.Scene.grit_gcols_convert = BoolProperty(name="Convert tcol -> gcol", description="Whether or not to run the external tool", default=True)

class SceneSummaryPanel(bpy.types.Panel): 
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

# {{{ additional object metadata


bpy.types.Object.grit_promotion = bpy.props.EnumProperty(name='Promotion', default='NONE', items=promotion_enum_items)
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
            #o.grit_promotion = self.promotion
        return {'FINISHED'}

class InstantiateSelected(bpy.types.Operator):
    '''Instantiate the Grit Class (active object) at cursor'''
    bl_idname = "grit.instantiate"
    bl_label = "Instantiate Class"

    @classmethod
    def poll(cls, context):
        return context.active_object != None and context.active_object.grit_promotion == "CLASS"

    def execute(self, context):
        class_obj = context.active_object

        meshes = [c for c in class_obj.children if c.grit_promotion == 'MESH']

        if len(meshes) == 0:
            self.report({'ERROR_INVALID_INPUT'}, "Class has no graphical representation (mesh)")
            return {'FINISHED'}
        if len(meshes) > 1:
            self.report({'ERROR_INVALID_INPUT'}, "Class has more than 1 graphical representation (mesh)")
            return {'FINISHED'}

        child = meshes[0]

        me = child.data

        bpy.ops.object.add(type="MESH", location=context.scene.cursor_location)
        ob = context.active_object
        old_data = ob.data
        ob.data = me
        bpy.data.meshes.remove(old_data)

        ob.grit_promotion = "OBJECT"
        ob.grit_object_class_name = strip_leading_exc(class_obj.name)

        # TODO: copy modifiers

        bpy.ops.object.select_all(action="DESELECT")
        ob.select = True
        return {'FINISHED'}
#}}}


def to_lua (v):
    if v == True: return "true"
    if v == False: return "false"
    if type(v) == type("a string"): return repr(v)
    return str(v)


def export_objects (scene, objs):
    errors = []

    if scene.grit_map_export:
        f = open(scene.grit_map_file, "w")
        f.write("-- Lua file generated by Blender map export script.\n")
        f.write("-- WARNING: If you modify this file, your changes will be lost if it is subsequently re-exported from blender\n\n")

        for obj in objs:
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
            if obj.grit_promotion == 'CLASS':
                class_name = strip_leading_exc(obj.name)

                meshes = [c for c in obj.children if c.grit_promotion == 'MESH']
                cols = [c for c in obj.children if c.grit_promotion == 'GCOL']

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

    for obj in objs:
        if obj.grit_promotion == 'MESH':
            filename = obj.name+".xml"
            the_mesh = obj.to_mesh(scene, True, "PREVIEW")
            export_mesh_xml(the_mesh, filename)
        if obj.grit_promotion == 'GCOL':
            filename = obj.name
            the_mesh = obj.to_mesh(scene, True, "PREVIEW")
            export_mesh_tcol(the_mesh, filename)

    # clip errors at a max to avoid filling the screen with crap
    max_errors = 20
    if len(errors) > max_errors: errors = errors[:max_errors] + ["..."]
    if len(errors) > 0: return "\n".join(errors)



class ScenePanel(bpy.types.Panel): #{{{
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


class MyProps:
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_context = "objectmode"

class ObjectPanel(bpy.types.Panel, MyProps):
    bl_label = "Grit Object Properties"

    def draw(self, context):
        obj = context.active_object
        if obj == None: return
        row = self.layout

        if obj.grit_promotion != "OBJECT":
            row.enabled = False

        row.prop(obj, "grit_object_class_name")
        row.operator("grit.apply_class")

class ClassPanel(bpy.types.Panel, MyProps):
    bl_label = "Grit Class Properties"

    def draw(self, context):
        obj = context.active_object
        if obj == None: return
        row = self.layout

        if obj.grit_promotion != "CLASS":
            row.enabled = False

        #row.template_list(context.scene, "objects", context.scene, "blah")

        row.prop(obj, "grit_class_rendering_distance")
        row.prop(obj, "grit_class_cast_shadows")
        row.prop(obj, "grit_class_place_z_off")
        row.prop(obj, "grit_class_place_rnd_rot")

        row.operator("grit.instantiate")
        

class SelectedPanel(bpy.types.Panel):
    bl_label = "Apply to selected"
    #bl_space_type = 'PROPERTIES'
    #bl_region_type = 'WINDOW'
    #bl_context = "object"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_context = "objectmode"

    def draw(self, context):
        sel_objs = context.selected_objects

        row = self.layout.row(align=True)
        row.alignment = "EXPAND"
        op = row.operator("grit.promote", text="Promotion")
        row.prop(context.scene, "grit_selected_promotion", text="")



def register():
    bpy.utils.register_module(__name__)

def unregister():
    bpy.utils.unregister_module(__name__)
