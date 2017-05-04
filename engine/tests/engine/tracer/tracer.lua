gfx_colour_grade `neutral.lut.png`
gfx_fade_dither_map `stipple.png`
gfx_particle_ambient(vec(1, 1, 1))

cam_pos = vec(0, 0, 0)
cam_look = vec(0, 100, 0)
cam_quat = quat(vec(0, 1, 0), cam_look - cam_pos)

b1 = gfx_tracer_body_make()
b1.texture = `Tracer.png`
b1.length = 30
b1.size = 0.5
b1.alpha = 0;

for i = 0, 5000, 10 do
    b1.localPosition = vec(math.sin(math.rad(i)), -10 + i / 5000 * 50, math.cos(math.rad(i)))
    b1.emissiveColour = (b1.localPosition + vec(1, 0, 1)) * vec(0.5, 0, 0.5) + vec(0, i / 5000, 0)
    gfx_render(0.1, cam_pos, cam_quat)
    gfx_tracer_body_pump(0.1)
end

gfx_screenshot('output-tracer1.png')


b2 = gfx_tracer_body_make()
b2.length = 10
b2.size = 0.2
b2.alpha = 0.75
b2.diffuseColour = vec(1, 0, 0)

function p(x, z)
    b2.localPosition = vec(x, 3, z)
    b2:pump()
end

p(0, 1)
p(0.5, 1)
p(1, 1)
p(1, 0.5)
p(1, 0)
p(0.5, 0)
p(0, 0)
p(0, 0.5)


p(0.5, 0.5)
p(0.5, 0.5)
p(0.5, 0.6)
p(0.5, 0.7)

gfx_render(0.1, cam_pos, cam_quat)
gfx_render(0.1, cam_pos, cam_quat)
gfx_render(0.1, cam_pos, cam_quat)

gfx_screenshot('output-tracer2.png')

