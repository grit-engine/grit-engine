--mlockall()

add_resource_location("gtasa","FileSystem",false)
if not gtasa_no_col then
        include("gtasa/all_col.lua")
end
initialise_all_resource_groups()

include("lua_base/map_classes.lua")
if gtasa_no_col then
        ColClass = BaseClass
end

if pw==nil or not alive(pw) or pw~=physics.world then
        pw = physics.world
end

if gom==nil then
        gom = get_gom()
end

if sm==nil or not alive(sm) or sm~=gfx.scnmgr then
        sm = gfx.scnmgr
        sm.shadowTechnique = "NONE"
        gfx.cam.nearClipDistance = 0.4
        gfx.cam.farClipDistance = 1200
        gfx.queueAggressiveness = function () return 1 + get_in_queue_size() * 0.5 end
        sm:setFog("EXP2",0.7137,0.5333,0.4,0.0015,0,0)
        sm:setAmbientLight(0.3,0.3,0.3)
        if sun==nil or not alive(sun) then
                sun = sm:createLight("Sun")
                sun.type = "DIRECTIONAL"
                sun.direction = Vector3(.1,1,-1).normalised
                sun:setDiffuseColour(.7,.7,.7)
                sun:setSpecularColour(1,1,1)
        end
end

movement.speedFast = 80
movement.speedSlow = 35

set_mesh_budget(32*1024*1024)
set_texture_budget(32*1024*1024)

include "gtasa/mat.lua"

gom:clearClasses()
gom:clearObjects()
include "gtasa/grit_classes.lua"
include "physics_assets/test_classes.lua"
include "gtasa/map.lua"

gom.stepSize = 25000


function go_la()
        gfx:warp(Vector3(2312.48, -1621.77, 35.7562), Quat(0.870673,0.00683816,0.00386254,0.4918))
end

function go_vegas()
        gfx:warp(Vector3(1856.87, 1360.64, 60.1808), Quat(0.981828,-0.0473323,-0.00884934,0.183564))
end

function go_vegas2()
        gfx:warp(Vector3(1770.23, 2959.79, 41.6458), Quat(0.00732922,-0.000130489,-0.0178007,0.999815))
end

function go_sf()
        gfx:warp(Vector3(-2016.56, 884.951, 68.9321), Quat(0.868686,-0.051001,0.0288787,-0.491883))
end

function go_nowhere()
        gfx:warp(Vector3(0,0,-10000))
end



-- vim: shiftwidth=8:tabstop=8:expandtab
