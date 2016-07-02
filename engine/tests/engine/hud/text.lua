include `font_impact50.lua`
gfx_colour_grade(`neutral.lut.png`)
gfx_option('POST_PROCESSING', false)
t = gfx_hud_text_add(`Impact50`)
t.text = "Some test text 123"
t.position = vec(200, 100)
gfx_render(0.1, vec(0, 0, 0), quat(1, 0, 0, 0))
gfx_screenshot('output-text.png')
