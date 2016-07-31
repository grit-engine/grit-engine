gfx_colour_grade(`neutral.lut.png`)
gfx_option('POST_PROCESSING', false)

gfx_hud_class_add(`Rect`, {
})

function rect(coord, fmt)
    obj = gfx_hud_object_add(`Rect`)
    obj.position = vec(128, 128) * (coord + vec(0.5, 0.5))
    obj.texture = `all_dds.` .. fmt .. ".dds"
    obj.size = vec(128, 128)
end

rect(vec(0, 0), "R5G6B5")
rect(vec(0, 1), "R8G8B8")
rect(vec(0, 2), "A8R8G8B8")
rect(vec(0, 3), "A2R10G10B10")
rect(vec(1, 0), "A1R5G5B5")
rect(vec(1, 1), "R8")
rect(vec(1, 2), "R16")
rect(vec(1, 3), "G16R16")
-- rect(vec(2, 0), "A8R8")
rect(vec(2, 1), "A4R4")
--rect(vec(2, 2), "A16R16")
rect(vec(2, 3), "R3G3B2")
rect(vec(3, 0), "A4R4G4B4")
rect(vec(3, 1), "BC1")
rect(vec(3, 2), "BC2")
rect(vec(3, 3), "BC3")
--rect(vec(4, 0), "BC4")
--rect(vec(4, 1), "BC5")

gfx_render(0.1, vec(0, 0, 0), quat(1, 0, 0, 0))
gfx_screenshot('output-all-dds.png')
