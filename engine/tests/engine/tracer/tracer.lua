gfx_colour_grade `neutral.lut.png`
gfx_fade_dither_map `stipple.png`

b1 = gfx_tracer_body_make()
b1.texture = `Tracer.png`
b1.length = 30
b1.size = 0.3

for i = 0, 359, 10 do
    b1.localPosition = vec(math.sin(math.rad(i)), 5, math.cos(math.rad(i)))
    b1.emissiveColour = (b1.localPosition + vec(1, 0, 1)) / vec(2, 1, 2)
    gfx_tracer_body_pump()
end

gfx_render(0.1, vec(0, 0, 1), quat(1, 0, 0, 0))
gfx_render(0.1, vec(0, 0, 1), quat(1, 0, 0, 0))
gfx_render(0.1, vec(0, 0, 1), quat(1, 0, 0, 0))

gfx_screenshot('output-tracer.png')
