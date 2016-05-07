gfx_colour_grade(`neutral.lut.png`)
gfx_option('POST_PROCESSING', true)
function dump(x)
    return tostring(x)
end
gfx_render(0.1, vec(0, 0, 0), quat(1, 0, 0, 0))
gfx_screenshot('output.png')
