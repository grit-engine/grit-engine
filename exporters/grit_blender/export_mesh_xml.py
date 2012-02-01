__author__ = ["Dave Cunningham"]
__version__ = '1.0'
__bpydoc__ = """\
Write some documentation.
"""

import bpy


def float_eq (x, y):
    # no reason to handle rounding errors at this point
    return x == y

def uv_eq (x, y):
    return float_eq(x[0], y[0]) and float_eq(x[1], y[1])

def export_mesh(mesh, filename):
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



def export_obj(filepath, scene, obj):
    export_mesh(filepath, obj.to_mesh(scene, True, "PREVIEW"))

