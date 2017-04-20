gfx_colour_grade(`neutral.lut.png`)
gfx_option('POST_PROCESSING', false)

gfx_hud_class_add(`Rect`, {
})

obj = gfx_hud_object_add(`Rect`)
obj.position = vec(300, 300)  -- centre of object, from screen bottom left
obj.size = vec(400, 300)
obj.stencil = true
obj.stencilTexture = `speech-bubble.png`
obj.orientation = 30
obj.colour = vec(0, 1, 0)
obj.zOrder = 5

obj2 = gfx_hud_object_add(`Rect`)
obj2.parent = obj
obj2.size = vec(400, 100)
obj2.colour = vec(1, 0, 0)
obj2.orientation = 30


include `font_impact50.lua`

t = gfx_hud_text_add(`Impact50`)
t.parent = obj
t.position = vec(70, 0)
t.orientation = -30
t.zOrder = 5
t.text = [[The quick brown fox jumped over the lazy dog.
The quick brown fox jumped over the lazy dog.
The quick brown fox jumped over the lazy dog.
]]



obj3 = gfx_hud_object_add(`Rect`)
obj3.position = vec(600, 400)  -- centre of object, from screen bottom left
obj3.size = vec(400, 300)
obj3.stencil = true
obj3.orientation = 38
obj3.colour = vec(0, 0, 0.5)

obj4 = gfx_hud_object_add(`Rect`)
obj4.parent = obj3
obj4.size = vec(400, 300)
obj4.colour = vec(0.5, 0, 0.5)
obj4.orientation = 32


include `font_impact50.lua`

t2 = gfx_hud_text_add(`Impact50`)
t2.parent = obj3
t2.position = vec(70, 0)
t2.orientation = -30
t2.zOrder = 5
t2.text = [[The quick brown fox jumped over the lazy dog.
The quick brown fox jumped over the lazy dog.
The quick brown fox jumped over the lazy dog.
]]
t2.colour = vec(0, 0, 0)



gfx_render(0.1, vec(0, 0, 0), quat(1, 0, 0, 0))

gfx_render(0.1, vec(0, 0, 0), quat(1, 0, 0, 0))
gfx_screenshot('output-stencil.png')
