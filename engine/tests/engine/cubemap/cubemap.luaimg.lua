
longitude_planes = {}
for longitude = 0, 359 do
    longitude_planes[longitude] = vec3(sin(rad(longitude)), cos(rad(longitude)), 0)
end
function v3_to_colour(pos)
    pos = norm(pos)
    pos = quat(23.5, vec(0, 1, 0)) * pos
    local polar = vec2(
        deg(mod(atan2(pos.x, pos.y) + 2*math.pi, 2*math.pi)),
        deg(atan(pos.z / #pos.xy))
    )
    for longitude = 0, 359, 15 do
        if abs(dot(pos, longitude_planes[longitude])) < 0.01 then
            return vec3(0, 0, 0)
        end
    end
    for latitude = -180+15, 179, 15 do
        if abs(polar.y / 15 - floor(polar.y / 15)) < 0.08 then
            return vec3(0, 0, 0)
        end
    end
    local bg = vec(0.5, 0.5, 0.5)
    if polar.y > 0 then bg = bg + vec3(0, 0, 0.3) end
    if polar.x > 180 then bg = bg + vec3(0, 0.3, 0) end
    return bg
end;

local sz = 256
pos_x = mipmaps(make(vec(sz, sz), 3, function(p_) local p = p_/(sz - 1) * 2 - 1; return v3_to_colour(vec3( 1, p.y, -p.x)) end))
neg_x = mipmaps(make(vec(sz, sz), 3, function(p_) local p = p_/(sz - 1) * 2 - 1; return v3_to_colour(vec3(-1, p.y, p.x)) end))
pos_y = mipmaps(make(vec(sz, sz), 3, function(p_) local p = p_/(sz - 1) * 2 - 1; return v3_to_colour(vec3(p.x,  1, -p.y)) end))
neg_y = mipmaps(make(vec(sz, sz), 3, function(p_) local p = p_/(sz - 1) * 2 - 1; return v3_to_colour(vec3(p.x, -1, p.y)) end))
pos_z = mipmaps(make(vec(sz, sz), 3, function(p_) local p = p_/(sz - 1) * 2 - 1; return v3_to_colour(vec3(p.x, p.y,  1)) end))
neg_z = mipmaps(make(vec(sz, sz), 3, function(p_) local p = p_/(sz - 1) * 2 - 1; return v3_to_colour(vec3(-p.x, p.y, -1)) end))
dds_save_cube("cube.dds", "R8G8B8", pos_x, neg_x, pos_y, neg_y, pos_z, neg_z)
