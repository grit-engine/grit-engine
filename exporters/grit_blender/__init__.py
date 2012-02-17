# TODO
#
# instantiate external class
# point to an existing .mesh
# point to an existing .tcol
# prepend prefix to all names
# vertex painting (ambient)
# vertex painting (diffuse)
# vertex painting (blend)
#
# special classes:
# * piles
#
# special class attributes
# * health / damage threshold
# * colour specification (for paint)
# * explodes, explodeRadius, explodeDeactivate?
# * material maps
#
# xml import:
# * http://docs.python.org/library/xml.etree.elementtree.html

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
import bpy.path
import os.path
import inspect
import subprocess
from bpy.props import *
from mathutils import Quaternion, Vector


def strip_leading_exc(x):
    return x[1:] if x[0]=='!' else x


class MessageOperator(bpy.types.Operator):
    bl_idname = "error.message"
    bl_label = "Message"
    message = StringProperty()
    title = StringProperty(default="ERROR!")
    icon = StringProperty(default="ERROR")
 
    def execute(self, context):
        return {'FINISHED'}
 
    def invoke(self, context, event):
        wm = context.window_manager
        return wm.invoke_popup(self)
 
    def draw(self, context):
        row = self.layout.row()
        row.alignment = "CENTER"
        row.label(self.title, icon=self.icon)
        for line in self.message.split("\n"):
            self.layout.label(line)

def error_msg(msg, title="ERROR!", icon="ERROR"):
    if msg == None: return
    bpy.ops.error.message('INVOKE_DEFAULT', message=msg, title=title, icon=icon)

# {{{ mesh examination utility

def float_eq (x, y):
    # no reason to handle rounding errors at this point
    return x == y

def uv_eq (x, y):
    return float_eq(x[0], y[0]) and float_eq(x[1], y[1])


def get_vertexes_faces(mesh, no_normals, default_material):
    num_uv = len(mesh.uv_textures)
    class Empty: pass

    # list of vertexes with their attributes
    vertexes = []
    # table mapping material name to a list of face triples
    faces = { }

    counter = 0

    for fi, f in enumerate(mesh.faces):

        matname = default_material if len(mesh.materials)==0 else mesh.materials[f.material_index].name

        triangles = []

        tri = ([f.vertices[0], f.vertices[1], f.vertices[2]], [0,1,2])
        triangles.append(tri)

        if len(f.vertices) == 4:
            tri = ([f.vertices[0], f.vertices[2], f.vertices[3]], [0,2,3])
            triangles.append(tri)

        for triangle in triangles:
            face = [0,0,0]
            for fvi, vi in enumerate(triangle[0]):
                v = mesh.vertices[vi]
                vert = Empty()
                vert.pos =  v.co
                if no_normals:
                    vert.normal = Vector((0,0,0))
                else:
                    vert.normal = v.normal
                if num_uv == 0:
                    vert.uv = [0,0]
                else:
                    vert.uv = mesh.uv_textures[0].data[fi].uv[triangle[1][fvi]]

                def tostr (v):
                    return "(pos="+str(v.pos) + ", normal="+str(v.normal)+", uv="+str(v.uv)+")"

                # see if we already hvae a vertex that is close enough
                duplicate = None
                for evi, evert in enumerate(vertexes): #existing vertex id
                    if (evert.pos - vert.pos).length < 0.00001 and \
                       (evert.normal - vert.normal).length < 0.00001 and \
                       uv_eq(evert.uv, vert.uv):
                        duplicate = evi
                        break

                if duplicate != None:
                    face[fvi] = duplicate
                else:
                    vertexes.append(vert)
                    face[fvi] = counter
                    counter += 1
            if not matname in faces.keys():
                # first face we have seen of this material
                faces[matname] = []
            faces[matname].append(face)

    return (vertexes, faces)

#}}}


#{{{ Scene stuff             

promotion_enum_items = [
    ('NONE','No Export','Not promoted'),
    ('OBJECT','Object','Grit Object'),
    ('CLASS','Class','Grit Class'),         # for dummies only
    ('MESH','.mesh','Grit .mesh'),
    ('GCOL','col','Grit col'),              # for dummies or mesh (trimesh)
    ('PRIM','col primitive','Grit col primitive'),
    ('HULL','col hull','Grit col hull'),
]

bpy.types.Scene.grit_selected_promotion = bpy.props.EnumProperty(name='Grit Object Promotion', default='NONE', items=promotion_enum_items)

bpy.types.Scene.grit_map_file = StringProperty(name="Grit map path", description="Path of object placements lua file", maxlen= 1024, default= "map.lua")
bpy.types.Scene.grit_map_export = BoolProperty(name="Export map", description="Whether or not to emit an object placements lua file", default=True)
bpy.types.Scene.grit_classes_file = StringProperty(name="Grit classes path", description="Path of class definitions lua file", maxlen=1024, default= "classes.lua")
bpy.types.Scene.grit_classes_export = BoolProperty(name="Export classes", description="Whether or not to emit a class definitions lua file", default=True)
bpy.types.Scene.grit_meshes_export = BoolProperty(name="Export .mesh", description="Whether or not to generate Grit .mesh files", default=True)
bpy.types.Scene.grit_meshes_convert = BoolProperty(name="Convert mesh.xml -> mesh", description="Whether or not to run the external tool", default=True)
bpy.types.Scene.grit_gcols_export = BoolProperty(name="Export .gcol", description="Whether or not to generate Grit .gcol files", default=True)
#bpy.types.Scene.grit_gcols_convert = BoolProperty(name="Convert tcol -> gcol", description="Whether or not to run the external tool", default=True)

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
        summary.label("selected gcol parts")
        summary.label(func('PRIM'))
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

bpy.types.Object.grit_mesh_tangents = bpy.props.BoolProperty(name='Support normal map?', default=False)

bpy.types.Object.grit_gcol_static = bpy.props.BoolProperty(name='Static?', default=True)
bpy.types.Object.grit_gcol_mass = bpy.props.FloatProperty(name='Mass', default=100.0)
bpy.types.Object.grit_gcol_linear_damping = bpy.props.FloatProperty(name='Linear damping', default=0.5)
bpy.types.Object.grit_gcol_angular_damping = bpy.props.FloatProperty(name='Angular damping', default=0.5)
bpy.types.Object.grit_gcol_linear_sleep_thresh = bpy.props.FloatProperty(name='Linear sleep thresh.', default=1)
bpy.types.Object.grit_gcol_angular_sleep_thresh = bpy.props.FloatProperty(name='Angular sleep thresh.', default=0.8)
bpy.types.Object.grit_gcol_prim_material = bpy.props.StringProperty(name='Material', default="/common/Stone")
bpy.types.Object.grit_gcol_prim_margin = bpy.props.FloatProperty(name='Margin', default=0.04)
bpy.types.Object.grit_gcol_prim_shrink = bpy.props.FloatProperty(name='Shrink', default=0.04)

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

def set_parent(context, child, parent):
    bpy.ops.object.select_all(action="DESELECT")
    child.select = True
    context.scene.objects.active = parent
    bpy.ops.object.parent_set(type='OBJECT')

class NewClass(bpy.types.Operator):
    '''Create a new class at the cursor'''
    bl_idname = "grit.new_class"
    bl_label = "New Class"

    class_name = bpy.props.StringProperty(name="Class name", default="")
    collision = bpy.props.EnumProperty(name='Has .gcol?', default='MESH', items=[
        ('NONE','No .gcol','No .gcol'),
        ('EMPTY','Empty .gcol (e.g. for dynamic)','No collision mesh, only child parts / hulls'),
        ('MESH','Trimesh .gcol (mainly for static)','Has collision mesh, and optional child parts / hull'),
    ])


    def invoke(self, context, event):
        wm = context.window_manager
        return wm.invoke_props_dialog(self)

    def execute(self, context):

        class_name = self.class_name
        mesh_name = self.class_name + ".mesh"
        gcol_name = self.class_name + ".gcol"

        if class_name == "":
            error_msg("Must give name of class")
            return {'FINISHED'}

        if context.scene.objects.get(class_name) != None:
            error_msg("Object already exists with name \""+class_name+"\"")
            return {'FINISHED'}

        if context.scene.objects.get(mesh_name) != None:
            error_msg("Object already exists with name \""+mesh_name+"\"")
            return {'FINISHED'}

        if self.collision!="NONE" and context.scene.objects.get(gcol_name) != None:
            error_msg("Object already exists with name \""+gcol_name+"\"")
            return {'FINISHED'}

        pos = context.scene.cursor_location

        bpy.ops.object.add(type="EMPTY", location=pos)
        the_class = context.active_object
        the_class.name = "!"+class_name
        the_class.grit_promotion = "CLASS"

        bpy.ops.mesh.primitive_cube_add(location=pos+Vector((5,0,0)))
        the_mesh = context.active_object
        the_mesh.name = mesh_name
        the_mesh.grit_promotion = "MESH"
        set_parent(context, the_mesh, the_class)

        if self.collision!="NONE":
            if self.collision=="EMPTY":
                bpy.ops.object.add(type="EMPTY", location=pos+Vector((-5,0,0)))
                the_gcol = context.active_object
            else:
                bpy.ops.mesh.primitive_cube_add(location=pos+Vector((-5,0,0)))
                the_gcol = context.active_object
            the_gcol.name = gcol_name
            the_gcol.grit_promotion = "GCOL"
            set_parent(context, the_gcol, the_class)
        
        bpy.ops.object.select_all(action="DESELECT")
        the_class.select = True
        context.scene.objects.active = the_class

        return {'FINISHED'}

class NewPrimitive(bpy.types.Operator):
    '''Create a new class at the cursor'''
    bl_idname = "grit.new_primitive"
    bl_label = "New Primitive"

    @classmethod
    def poll(cls, context):
        return context.active_object != None and context.active_object.grit_promotion == "GCOL"

    def execute(self, context):
        gcol = context.active_object

        bpy.ops.mesh.primitive_cube_add(location=context.scene.cursor_location)
        ob = context.active_object
        ob.grit_promotion = "PRIM"
        set_parent(context, ob, gcol)

        bpy.ops.object.select_all(action="DESELECT")
        ob.select = True
        context.scene.objects.active = ob

        return {'FINISHED'}

class InstantiateSelected(bpy.types.Operator):
    '''Instantiate the Grit Class (active object) at cursor'''
    bl_idname = "grit.new_object"
    bl_label = "Instantiate Class"

    @classmethod
    def poll(cls, context):
        return context.active_object != None and context.active_object.grit_promotion == "CLASS"

    def execute(self, context):
        class_obj = context.active_object

        meshes = [c for c in class_obj.children if c.grit_promotion == 'MESH']

        if len(meshes) == 0:
            error_msg("Class has no graphical representation (mesh)")
            return {'FINISHED'}
        if len(meshes) > 1:
            error_msg("Class has more than 1 graphical representation (mesh)")
            return {'FINISHED'}

        child = meshes[0]

        ob = child.copy()
        for i in ob.keys(): del ob[i]
        ob.parent = None
        ob.name = "obj"
        context.scene.objects.link(ob)

        ob.grit_promotion = "OBJECT"
        ob.grit_object_class_name = strip_leading_exc(class_obj.name)
        ob.location = context.scene.cursor_location

        bpy.ops.object.select_all(action="DESELECT")
        ob.select = True
        context.scene.objects.active = ob
        return {'FINISHED'}
#}}}


def to_lua (v):
    if v == True: return "true"
    if v == False: return "false"
    if type(v) == type("a string"): return repr(v)
    return str(v)


def export_objects (scene, objs):
    errors = []

    grit_objects = [ x for x in objs if x.grit_promotion == 'OBJECT' ]
    grit_classes = [ x for x in objs if x.grit_promotion == 'CLASS' ]

    if scene.grit_map_export and len(grit_objects) > 0:
        f = open(scene.grit_map_file, "w")
        f.write("-- Lua file generated by Blender map export script.\n")
        f.write("-- WARNING: If you modify this file, your changes will be lost if it is subsequently re-exported from blender\n\n")

        for obj in grit_objects:
            if  obj.grit_object_class_name == "":
                errors.append("Object without class: \""+obj.name+"\"")
            else:
                pos = obj.location
                rot = obj.rotation_euler.to_quaternion()
                rot_str = rot == Quaternion((1,0,0,0)) and "" or "rot=quat("+str(rot.w)+", "+str(rot.x)+", "+str(rot.y)+", "+str(rot.z)+"), "
                class_name = obj.grit_object_class_name
                f.write("object \""+class_name+"\" ("+str(pos.x)+", "+str(pos.y)+", "+str(pos.z)+") { "+rot_str+"name=\""+obj.name+"\" }\n")

        f.close()

    if scene.grit_classes_export and len(grit_classes) > 0:
        f = open(scene.grit_classes_file, "w")
        f.write("-- Lua file generated by Blender class export script.\n")
        f.write("-- WARNING: If you modify this file, your changes will be lost if it is subsequently re-exported from blender\n\n")

        for obj in grit_classes:
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
            attributes.append(("placementZOffset", obj.grit_class_place_z_off))
            attributes.append(("placementRandomRotation", obj.grit_class_place_rnd_rot))

            if class_name + ".mesh" != meshes[0].name: attributes.append(("gfxMesh", meshes[0].name))
            parent_class_name = ""
            if len(cols) == 0:
                parent_class_name = "BaseClass"
                interior = " "
            else:
                parent_class_name = "ColClass"
                if len(cols) > 1:
                    errors.append("Class has "+len(meshes)+" cols (should have 0 or 1): \""+obj.name+"\"")
                    continue
                if class_name + ".gcol" != cols[0].name: attributes.append(("colMesh", cols[0].name))

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
            mesh = obj.to_mesh(scene, True, "PREVIEW")

            if len(mesh.materials) == 0:
                errors.append("Grit mesh \""+obj.name+"\" has no materials")

            (vertexes, faces) = get_vertexes_faces(mesh, False, "/common/White")

            file = open(filename, "w")
            file.write("<mesh>\n")
            file.write("    <sharedgeometry>\n")
            file.write("        <vertexbuffer positions=\"true\" normals=\"true\" texture_coord_dimensions_0=\"float2\" texture_coords=\"1\">\n")
            for v in vertexes:
                file.write("            <vertex>\n")
                file.write("                <position x=\""+str(v.pos.x)+"\" y=\""+str(v.pos.y)+"\" z=\""+str(v.pos.z)+"\" />\n")
                file.write("                <normal x=\""+str(v.normal.x)+"\" y=\""+str(v.normal.y)+"\" z=\""+str(v.normal.z)+"\" />\n")
                file.write("                <texcoord u=\""+str(v.uv[0])+"\" v=\""+str(-v.uv[1])+"\" />\n")
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

            if scene.grit_meshes_convert:
                current_py = inspect.getfile(inspect.currentframe())
                exporter = os.path.abspath(os.path.dirname(current_py) + "/OgreXMLConverter")
                subprocess.call([exporter, "-e", "-t", "-ts", "4", filename])

        if obj.grit_promotion == 'GCOL':
            filename = obj.name

            file = open(filename, "w")
            file.write("TCOL1.0\n")
            file.write("\n")
            file.write("attributes {\n")
            if obj.grit_gcol_static:
                file.write("    static;\n")
            else:
                file.write("    mass "+str(obj.grit_gcol_mass)+";\n")
            file.write("    linear_damping "+str(obj.grit_gcol_linear_damping)+";\n")
            file.write("    angular_damping "+str(obj.grit_gcol_angular_damping)+";\n")
            file.write("    linear_sleep_threshold "+str(obj.grit_gcol_linear_sleep_thresh)+";\n")
            file.write("    angular_sleep_threshold "+str(obj.grit_gcol_angular_sleep_thresh)+";\n")
            file.write("}\n")
            file.write("\n")

            file.write("compound {\n")
            for c in obj.children:
                file.write("    // "+c.name+"\n")
                mesh = c.data
                pos = c.matrix_local.to_translation()
                rot = c.matrix_local.to_quaternion()
                scale = c.matrix_local.to_scale()
                mat = c.grit_gcol_prim_material
                marg = c.grit_gcol_prim_margin
                shrink = c.grit_gcol_prim_shrink
                if mesh.name == "GritPhysicsBox":
                    file.write("    box {\n")
                    file.write("        material \""+mat+"\";\n")
                    file.write("        centre "+str(pos.x)+" "+str(pos.y)+" "+str(pos.z)+";\n")
                    file.write("        orientation "+str(rot.w)+" "+str(rot.x)+" "+str(rot.y)+" "+str(rot.z)+";\n")
                    file.write("        dimensions "+str(scale.x)+" "+str(scale.y)+" "+str(scale.z)+";\n")
                    file.write("        margin "+str(marg)+";\n")
                    file.write("    }\n")
                elif mesh.name == "GritPhysicsSphere":
                    file.write("    sphere {\n")
                    file.write("        material \""+mat+"\";\n")
                    file.write("        centre "+str(pos.x)+" "+str(pos.y)+" "+str(pos.z)+";\n")
                    file.write("        radius "+str((scale.x+scale.y+scale.z)/3)+";\n")
                    file.write("    }\n")
                elif mesh.name == "GritPhysicsCylinder":
                    file.write("    cylinder {\n")
                    file.write("        material \""+mat+"\";\n")
                    file.write("        centre "+str(pos.x)+" "+str(pos.y)+" "+str(pos.z)+";\n")
                    file.write("        orientation "+str(rot.w)+" "+str(rot.x)+" "+str(rot.y)+" "+str(rot.z)+";\n")
                    file.write("        dimensions "+str(scale.x)+" "+str(scale.y)+" "+str(scale.z)+";\n")
                    file.write("        margin "+str(marg)+";\n")
                    file.write("    }\n")
                elif mesh.name == "GritPhysicsCone":
                    file.write("    cone {\n")
                    file.write("        material \""+mat+"\";\n")
                    file.write("        centre "+str(pos.x)+" "+str(pos.y)+" "+str(pos.z)+";\n")
                    file.write("        orientation "+str(rot.w)+" "+str(rot.x)+" "+str(rot.y)+" "+str(rot.z)+";\n")
                    file.write("        height "+str(scale.z)+";\n")
                    file.write("        radius "+str((scale.x+scale.y)/2)+";\n")
                    file.write("        margin "+str(marg)+";\n")
                    file.write("    }\n")
                else:
                    mesh = c.to_mesh(scene, True, "PREVIEW")
                    (vertexes, faces) = get_vertexes_faces(mesh, True, "DEFAULT")
                    file.write("    hull {\n")
                    file.write("        material \""+mat+"\";\n")
                    file.write("        margin "+str(marg)+";\n")
                    file.write("        shrink "+str(shrink)+";\n")
                    file.write("        vertexes {\n")
                    for v in vertexes:
                        p = c.matrix_local * v.pos
                        file.write("            "+str(p.x)+" "+str(p.y)+" "+str(p.z)+";\n")
                    file.write("        }\n")
                    file.write("    }\n")
            file.write("}\n")

            if obj.type == "MESH":
                # triangles to add
                mesh = obj.to_mesh(scene, True, "PREVIEW")

                if len(mesh.materials) == 0:
                    errors.append("Grit gcol \""+obj.name+"\" has no materials")

                (vertexes, faces) = get_vertexes_faces(mesh, True, "/common/Stone")
                file.write("trimesh {\n")
                file.write("    vertexes {\n")
                for v in vertexes:
                    file.write("        "+str(v.pos.x)+" "+str(v.pos.y)+" "+str(v.pos.z)+";\n")
                file.write("    }\n")
                file.write("    faces {\n")
                for m in faces.keys():
                    for f in faces[m]:
                        file.write("        "+str(f[0])+" "+str(f[1])+" "+str(f[2])+" \""+m+"\";\n")
                file.write("    }\n")
                file.write("}\n")

            file.close()

    # clip errors at a max to avoid filling the screen with crap
    max_errors = 20
    if len(errors) > max_errors: errors = errors[:max_errors] + ["..."]
    if len(errors) > 0: return "\n".join(errors)



class ScenePanel(bpy.types.Panel): #{{{
    bl_label = "Grit Export Settings"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'

    #@classmethod
    #def poll(cls, context):
    #    return len(context.selected_objects) == 1

    def draw(self, context):
        box = self.layout
        col = box.column(align=True)
        row = col.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_map_export", text="Export?")
        row.prop(context.scene, "grit_map_file", text="", expand=True)
        row = col.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_classes_export", text="Export?")
        row.prop(context.scene, "grit_classes_file", text="", expand=True)
        row = col.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_meshes_export")
        row.prop(context.scene, "grit_meshes_convert", text="Convert", expand=True)
        row = col.row()
        row.alignment = "LEFT"
        row.prop(context.scene, "grit_gcols_export")
        #row.prop(context.scene, "grit_gcols_convert", text="Convert", expand=True)
        #row = box.row()
        col.operator("grit.export_selected", icon="SCRIPT")
        col.operator("grit.export_all", icon="SCRIPT")
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
        error_msg(err)
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
        error_msg(err)
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

class ToolsPanel(bpy.types.Panel):
    bl_label = "Grit Tools"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_context = "objectmode"

    def draw(self, context):
        sel_objs = context.selected_objects

        root = self.layout.box()
        root.label("New at cursor", icon="NEW")
        column = root.column(align=True)
        column.alignment = "EXPAND"
        op = column.operator("grit.new_class")
        op = column.operator("grit.new_object")
        op = column.operator("grit.new_primitive")

        obj = context.active_object
        if obj == None: return

        root = self.layout.box()
        root.label("Object", icon="OBJECT_DATAMODE")
        root = root.column()
        if obj.grit_promotion != "OBJECT":
            root.enabled = False
        root.prop(obj, "grit_object_class_name")
        root.operator("grit.apply_class", icon="HAND")

        root = self.layout.box()
        root.label("Class", icon="VIEW3D")
        root = root.column()
        if obj.grit_promotion != "CLASS":
            root.enabled = False
        #root.template_list(context.scene, "objects", context.scene, "blah")
        root.prop(obj, "grit_class_rendering_distance")
        root.prop(obj, "grit_class_cast_shadows")
        root.prop(obj, "grit_class_place_z_off")
        root.prop(obj, "grit_class_place_rnd_rot")

        root = self.layout.box()
        root.label(".gcol", icon="PHYSICS")
        root = root.column()
        if obj.grit_promotion != "GCOL":
            root.enabled = False
        #root.template_list(context.scene, "objects", context.scene, "blah")
        root.prop(obj, "grit_gcol_static")
        r = root.column()
        r.prop(obj, "grit_gcol_mass")
        if obj.grit_gcol_static: r.enabled = False
        r.prop(obj, "grit_gcol_linear_damping")
        if obj.grit_gcol_static: r.enabled = False
        r.prop(obj, "grit_gcol_angular_damping")
        if obj.grit_gcol_static: r.enabled = False
        r.prop(obj, "grit_gcol_linear_sleep_thresh")
        if obj.grit_gcol_static: r.enabled = False
        r.prop(obj, "grit_gcol_angular_sleep_thresh")
        if obj.grit_gcol_static: r.enabled = False
        
        root = self.layout.box()
        mapping = dict()
        mapping["GritPhysicsSphere"] = " (sphere)"
        mapping["GritPhysicsBox"] = " (box)"
        mapping["GritPhysicsCone"] = " (cone)"
        mapping["GritPhysicsCylinder"] = " (cylinder)"
        extra = obj.type == "MESH" and obj.grit_promotion == "PRIM" and (mapping.get(obj.data.name) or " (hull)") or ""
        root.label(".gcol prim"+extra, icon="PHYSICS")
        root = root.column()
        if obj.grit_promotion != "PRIM":
            root.enabled = False
        #root.template_list(context.scene, "objects", context.scene, "blah")
        root.prop(obj, "grit_gcol_prim_material")
        r = root.row()
        r.prop(obj, "grit_gcol_prim_margin")
        if obj.type == "MESH" and obj.data.name == "GritPhysicsSphere": r.enabled = False
        r = root.row()
        r.prop(obj, "grit_gcol_prim_shrink")
        if obj.type == "MESH" and obj.data.name != "GritPhysicsHull": r.enabled = False
        
def register():
    bpy.utils.register_module(__name__)

def unregister():
    bpy.utils.unregister_module(__name__)
