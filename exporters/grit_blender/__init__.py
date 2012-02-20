# TODO
#
# instantiate external class
# point to an existing .mesh
# point to an existing .tcol
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
# * lights
# * LOD
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
import os
import inspect
import subprocess
from bpy.props import *
from mathutils import Quaternion, Vector

executable_suffix = ".exe" if os.pathsep == "\\" else ""

def my_abspath(x):
    return os.path.abspath(bpy.path.abspath(x))

def path_to_os(x):
    return x.replace("/",os.pathsep).replace("\\",os.pathsep)

def path_to_grit(x):
    return x.replace("\\","/")

def grit_path_from_data(scene, data):
    if data.library == None: return data.name
    return grit_path(scene, data.library.filepath, data.name)

# assuming grit_root_dir is ../..
# takes e.g. "//../../common/lib.blend" and "mat/White"
# returns "/common/mat/White"
# or takes e.g. "//../base.blend" "Fish"
# returns "../Fish"
def grit_path(scene, remote_blend, remote_resource_name):
    root = my_abspath("//"+scene.grit_root_dir)
    this_blend_dir = my_abspath("//")
    that_blend = my_abspath(remote_blend)
    that_blend_dir = os.path.dirname(that_blend)
    if not this_blend_dir.startswith(root):
        error_msg("Invalid root directory: \""+scene.grit_root_dir+"\"\n(expands to \""+root+"\")\n does not contain \""+this_blend_dir+"\"")
        return "UNKNOWN"
    if not that_blend_dir.startswith(root):
        error_msg("Invalid root directory: \""+scene.grit_root_dir+"\"\n(expands to \""+root+"\")\n does not contain \""+that_blend+"\"")
        return "UNKNOWN"

    # make paths use game's root
    this_blend_dir = this_blend_dir[len(root):]
    that_blend_dir = that_blend_dir[len(root):]
    pref = os.path.commonprefix([this_blend_dir, that_blend_dir])

    if pref == "":
        # use absolute grit path, e.g. "/common/mat/White"
        return path_to_grit(that_blend_dir+"/"+remote_resource_name)
    else:
        # use path relative to current blend file,  e.g. "../Fish"
        return path_to_grit(os.path.relpath(that_blend_dir, this_blend_dir)+"/"+remote_resource_name)

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
        return wm.invoke_popup(self, width=500)
 
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


def get_vertexes_faces(scene, mesh, no_normals, default_material):
    num_uv = len(mesh.uv_textures)
    class Empty: pass

    # list of vertexes with their attributes
    vertexes = []
    # table mapping material name to a list of face triples
    faces = { }

    counter = 0

    for fi, f in enumerate(mesh.faces):

        matname = default_material if (len(mesh.materials)==0 or mesh.materials[f.material_index] == None) else grit_path_from_data(scene, mesh.materials[f.material_index])

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


bpy.types.Scene.grit_root_dir = StringProperty(name="Grit root dir", description="The root of the game directory (where Grit.exe lives)", maxlen= 1024, default= "..")

bpy.types.Scene.grit_map_export = BoolProperty(name="Export map", description="Whether or not to emit an object placements lua file", default=True)
bpy.types.Scene.grit_map_file = StringProperty(name="Grit map path", description="Path of object placements lua file", maxlen= 1024, default= "map.lua")
bpy.types.Scene.grit_map_prefix = StringProperty(name="Map obj prefix", description="This will be prepended to all of your object names, useful to avoid clashes between different blend files", maxlen= 1024, default= "my_")

bpy.types.Scene.grit_classes_export = BoolProperty(name="Export classes", description="Whether or not to emit a class definitions lua file", default=True)
bpy.types.Scene.grit_classes_file = StringProperty(name="Grit classes path", description="Path of class definitions lua file", maxlen=1024, default= "classes.lua")

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
        summary.label("selected gcol parts")
        summary.label(func('PRIM'))
        summary = box.row(align=True)
        summary.label("selected unpromoted")
        summary.label(func('NONE'))


#}}}

# {{{ additional object metadata


bpy.types.Object.grit_promotion = bpy.props.EnumProperty(name='Promotion', default='NONE', items=promotion_enum_items, description="The special role within the Grit engine that this Blender object represents")
bpy.types.Object.grit_object_class_name = bpy.props.StringProperty(name='Class', default="", description="Name of the class (without the !) of which this Grit object is an instance")

# piles

bpy.types.Object.grit_class_rendering_distance = bpy.props.FloatProperty(name='Rendering Distance', default=100, description="Distance at which the object is deactivated, e.g. no-longer rendered or generating collisions")
bpy.types.Object.grit_class_cast_shadows = bpy.props.BoolProperty(name='Casts Shadows?', default=False, description="Does the graphical representation of the object cast shadows within Grit")
bpy.types.Object.grit_class_place_z_off = bpy.props.FloatProperty(name='Placement Z Offset', default=0.0, description="When placing objects on the map using place(), the distance above ground to spawn (to avoid violent intersection)")
bpy.types.Object.grit_class_place_rnd_rot = bpy.props.BoolProperty(name='Placement Random Z Rotate?', default=False, description="When placing objects on the map using place(), whether a random rotation about Z is used (to introduce some variety)")

bpy.types.Object.grit_mesh_tangents = bpy.props.BoolProperty(name='Tangents?', default=False, description="Whether or not to generate tangents in the mesh file (takes more disk & memory but required when using normal maps)")

bpy.types.Object.grit_gcol_static = bpy.props.BoolProperty(name='Static?', default=True, description="Whether or not objects using this gcol are fixed in place (like a wall)")
bpy.types.Object.grit_gcol_mass = bpy.props.FloatProperty(name='Mass', default=100.0, description="The mass of the object in kilograms")
bpy.types.Object.grit_gcol_linear_damping = bpy.props.FloatProperty(name='Linear damping', default=0.5, description="Proportion of linear velocity lost per second due to drag / friction")
bpy.types.Object.grit_gcol_angular_damping = bpy.props.FloatProperty(name='Angular damping', default=0.5, description="Proportion of angular velocity lost per second due to drag / friction")
bpy.types.Object.grit_gcol_linear_sleep_thresh = bpy.props.FloatProperty(name='Linear sleep thresh.', default=1, description="Linear velocity below which object will sleep")
bpy.types.Object.grit_gcol_angular_sleep_thresh = bpy.props.FloatProperty(name='Angular sleep thresh.', default=0.8, description="Angular velocity below which object will sleep")
bpy.types.Object.grit_gcol_prim_margin = bpy.props.FloatProperty(name='Margin', default=0.04, description="Collsion margin -- distance after which a different collision resolution algorithm is used.  For hulls, forms an invisible shell around the object")
bpy.types.Object.grit_gcol_prim_shrink = bpy.props.FloatProperty(name='Shrink', default=0.04, description="For hulls, a distance the hull should be shrunk inwards along face normals to compensate for the margin")


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
        return wm.invoke_props_dialog(self, width=500)

    def execute(self, context):

        class_name = self.class_name
        mesh_name = self.class_name + ".mesh"
        gcol_name = self.class_name + ".gcol"

        if class_name == "":
            error_msg("Must give name of class")
            return {'FINISHED'}

        if context.scene.objects.get('!'+class_name) != None:
            error_msg("Object already exists with name \""+'!'+class_name+"\"")
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

class RenameClass(bpy.types.Operator):
    '''Rename the selected class'''
    bl_idname = "grit.rename_class"
    bl_label = "Rename Class"

    class_name = bpy.props.StringProperty(name="Class name", default="")
    rename_mesh = BoolProperty(name=".mesh follows class name", description="Whether to also rename the .mesh object after the class", default=True)
    rename_gcol = BoolProperty(name=".gcol follows class name", description="Whether to also rename the .gcol object after the class", default=True)

    def invoke(self, context, event):
        wm = context.window_manager
        return wm.invoke_props_dialog(self, width=500)

    @classmethod
    def poll(cls, context):
        return context.active_object != None and context.active_object.grit_promotion == "CLASS"

    def execute(self, context):
        class_obj = context.active_object

        old_class_name = strip_leading_exc(class_obj.name)

        class_name = self.class_name
        mesh_name = self.class_name + ".mesh"
        gcol_name = self.class_name + ".gcol"

        rename_gcol = False
        for c in class_obj.children:
            if c.grit_promotion == "GCOL": rename_gcol = True
        rename_gcol = rename_gcol and self.rename_gcol

        if class_name == "":
            error_msg("Must give name of class")
            return {'FINISHED'}

        if context.scene.objects.get('!'+class_name) != None:
            error_msg("Object already exists with name \""+'!'+class_name+"\"")
            return {'FINISHED'}

        if self.rename_mesh and context.scene.objects.get(mesh_name) != None:
            error_msg("Object already exists with name \""+mesh_name+"\"")
            return {'FINISHED'}

        if rename_gcol and context.scene.objects.get(gcol_name) != None:
            error_msg("Object already exists with name \""+gcol_name+"\"")
            return {'FINISHED'}

        for c in class_obj.children:
            if c.grit_promotion == "GCOL" and rename_gcol: c.name = gcol_name
            if c.grit_promotion == "MESH" and self.rename_mesh: c.name = mesh_name

        class_obj.name = '!'+class_name

        for obj in context.scene.objects:
            if obj.grit_promotion == "OBJECT" and obj.grit_object_class_name == old_class_name:
                obj.grit_object_class_name = class_name

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

    if bpy.path.abspath("//") == "":
        return "You must save your .blend file first.\nI do not know what the output directory is.\n"

    grit_objects = [ x for x in objs if x.grit_promotion == 'OBJECT' ]
    grit_classes = [ x for x in objs if x.grit_promotion == 'CLASS' ]

    if scene.grit_map_export and len(grit_objects) > 0:
        filename = my_abspath("//" + scene.grit_map_file)
        f = open(filename, "w")
        f.write("-- Lua file generated by Blender map export script.\n")
        f.write("-- WARNING: If you modify this file, your changes will be lost if it is subsequently re-exported from blender\n\n")

        for obj in grit_objects:
            if  obj.grit_object_class_name == "":
                errors.append("Object without class: \""+obj.name+"\"")
            else:
                pos = obj.location
                rot = obj.rotation_euler.to_quaternion()
                rot_str = rot == Quaternion((1,0,0,0)) and "" or "rot=quat("+str(rot.w)+", "+str(rot.x)+", "+str(rot.y)+", "+str(rot.z)+"), "
                #TODO: handle classes in other directories
                class_name = obj.grit_object_class_name
                f.write("object \""+class_name+"\" ("+str(pos.x)+", "+str(pos.y)+", "+str(pos.z)+") { "+rot_str+"name=\""+scene.grit_map_prefix+obj.name+"\" }\n")

        f.close()

    if scene.grit_classes_export and len(grit_classes) > 0:
        filename = my_abspath("//" + scene.grit_classes_file)
        f = open(filename, "w")
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

            #TODO: allow .mesh .gcol in subdirectories, other blend files, etc
            if class_name + ".mesh" != meshes[0].name: attributes.append(("gfxMesh", meshes[0].name))
            parent_class_name = ""
            if len(cols) == 0:
                parent_class_name = "BaseClass"
                interior = " "
            else:
                parent_class_name = "ColClass"
                if len(cols) > 1:
                    errors.append("Class has "+str(len(meshes))+" cols (should have 0 or 1): \""+obj.name+"\"")
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
            mesh = obj.to_mesh(scene, True, "PREVIEW")

            if len(mesh.materials) == 0:
                errors.append("Grit mesh \""+obj.name+"\" has no materials")

            for m in mesh.materials:
                if m == None:
                    errors.append("Grit mesh \""+obj.name+"\" has an undefined material")

            (vertexes, faces) = get_vertexes_faces(scene, mesh, False, "/system/FallbackMaterial")

            filename = my_abspath("//" + obj.name+".xml")
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
                exporter = os.path.abspath(os.path.join(os.path.dirname(current_py), "OgreXMLConverter" + executable_suffix))
                args = [exporter, "-e"]
                if obj.grit_mesh_tangents:
                    args.append("-t")
                    args.append("-ts")
                    args.append("4")
                args.append(filename)
                subprocess.call(args)

            bpy.data.meshes.remove(mesh)

        if obj.grit_promotion == 'GCOL':
            filename = my_abspath("//" + obj.name)
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
                if c.type == "EMPTY":
                    errors.append("Part \""+c.name+"\" under \""+obj.name+"\" is an Empty)")
                    continue
                file.write("    // "+c.name+"\n")
                mesh = c.data
                pos = c.matrix_local.to_translation()
                rot = c.matrix_local.to_quaternion()
                scale = c.matrix_local.to_scale()
                mat = "/system/FallbackPhysicalMaterial"
                if len(c.material_slots) == 0:
                    errors.append("Part \""+c.name+"\" under \""+obj.name+"\" has no materials)")
                else:
                    mat = c.material_slots[0].material
                    if mat == None:
                        mat = "/system/FallbackPhysicalMaterial"
                        errors.append("Part \""+c.name+"\" under \""+obj.name+"\" has undefined material")
                    else:
                        mat = grit_path_from_data(scene, mat)
                    if len(c.material_slots) > 1:
                        errors.append("Part \""+c.name+"\" under \""+obj.name+"\" must have 1 material (has "+str(len(c.material_slots))+")")
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
                    (vertexes, faces) = get_vertexes_faces(scene, mesh, True, "/system/FallbackPhysicalMaterial")
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
                    bpy.data.meshes.remove(mesh)
            file.write("}\n")

            if obj.type == "MESH":
                # triangles to add
                mesh = obj.to_mesh(scene, True, "PREVIEW")

                if len(mesh.materials) == 0:
                    errors.append("Grit gcol \""+obj.name+"\" has no materials")

                for m in mesh.materials:
                    if m == None:
                        errors.append("Grit .gcol \""+obj.name+"\" has undefined material")

                (vertexes, faces) = get_vertexes_faces(scene, mesh, True, "/common/Stone")

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

                bpy.data.meshes.remove(mesh)

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
        row = col.row(align=True)
        row.alignment = "EXPAND"
        row.label("Game root")
        row.prop(context.scene, "grit_root_dir", text="", expand=True)
        row = col.row(align=True)
        row.alignment = "EXPAND"
        row.prop(context.scene, "grit_map_export", text="Export?")
        row.prop(context.scene, "grit_map_file", text="", expand=True)
        row = col.row(align=True)
        row.alignment = "EXPAND"
        row.label("Obj prefix")
        row.prop(context.scene, "grit_map_prefix", text="", expand=True)
        row = col.row(align=True)
        row.alignment = "EXPAND"
        row.prop(context.scene, "grit_classes_export", text="Export?")
        row.prop(context.scene, "grit_classes_file", text="", expand=True)
        row = col.row(align=True)
        row.alignment = "EXPAND"
        row.prop(context.scene, "grit_meshes_export")
        row.prop(context.scene, "grit_meshes_convert", text="Convert", expand=True)
        row = col.row(align=True)
        row.alignment = "EXPAND"
        row.prop(context.scene, "grit_gcols_export")
        row = row.row()
        row.prop(context.scene, "grit_gcols_convert", text="Convert", expand=True)
        row.enabled = False
        row = box.row()
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
        op = column.operator("grit.rename_class")
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
        root.label(".mesh", icon="EDITMODE_HLT")
        root = root.column()
        if obj.grit_promotion != "MESH":
            root.enabled = False
        root.prop(obj, "grit_mesh_tangents")
        
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
        r = root.row()
        r.prop(obj, "grit_gcol_prim_margin")
        if obj.type == "MESH" and obj.data.name == "GritPhysicsSphere": r.enabled = False
        r = root.row()
        r.prop(obj, "grit_gcol_prim_shrink")
        if obj.type == "MESH" and obj.data.name == "GritPhysicsHull": r.enabled = False
        if obj.type == "MESH" and obj.data.name == "GritPhysicsSphere": r.enabled = False
        if obj.type == "MESH" and obj.data.name == "GritPhysicsBox": r.enabled = False
        if obj.type == "MESH" and obj.data.name == "GritPhysicsCone": r.enabled = False
        
def register():
    bpy.utils.register_module(__name__)

def unregister():
    bpy.utils.unregister_module(__name__)
