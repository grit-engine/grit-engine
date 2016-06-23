physics_set_material(`/common/pmat/Stone`, 4)  -- RoughGroup
gcol = `test.gcol`
hold = disk_resource_hold_make(gcol)  -- Keep it from being unloaded
disk_resource_ensure_loaded(gcol)  -- Load it (in rendering thread)

local body = physics_body_make(gcol, vec(0, 0, 0), quat(1, 0, 0, 0))

function do_test(ray_start, ray)
    local rad = 0.05
    local ray = vec(0, 0, -2)
    local dist = physics_sweep_sphere(rad, ray_start, ray, true, 0)
    local sweep_end = ray_start + dist * ray + rad * norm(ray)

    -- Compare with a simple ray
    dist = physics_cast(ray_start, ray, true, 0)
    local cast_end = ray_start + dist * ray

    return sweep_end - cast_end
end

print_stdout(do_test(vec(-150, 310, 1.5)))
print_stdout(do_test(vec(98, 14, 1.5)))
print_stdout(do_test(vec(192, -370, 1.5)))
print_stdout(do_test(vec(0, 0, 1.5)))

