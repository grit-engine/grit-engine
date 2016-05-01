gfx_colour_grade(`neutral.lut.png`)
gfx_option('POST_PROCESSING', true)
function dump(x)
    return tostring(x)
end
while not clicked_close() do
    gfx_render(0.1, vec(0, 0, 0), quat(1, 0, 0, 0))
end
gfx_screenshot('output.png')
