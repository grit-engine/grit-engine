
-- Used by the meshes.
register_material(`Small`, {
})

print "Loading Test.08.mesh" 
disk_resource_load(`Test.08.mesh`)
print "Using Test.08.mesh" 
b08 = gfx_body_make(`Test.08.mesh`)

print "Loading Test.08.mesh"
disk_resource_load(`Test.10.mesh`)
print "Using Test.08.mesh"
b10 = gfx_body_make(`Test.10.mesh`)
