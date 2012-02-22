Grit objects
type == "MESH"   (this is an object that links to a mesh)
data -> mesh whose name is !ClassName
name is the object's name

export a line to map file


Grit classes
name is the class name with ! on the front
type == "EMPTY"
Contains 1 or 2 children
One called X.mesh
-- this should have type == "MESH"
-- data -> mesh we export to X.mesh
-- name of mesh not very relevant
One called X.gcol
-- collision mesh
-- will be either type == "MESH" in which case standard mesh export
-- or type == "EMPTY" in which case it's a compound


