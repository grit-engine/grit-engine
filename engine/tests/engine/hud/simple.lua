gfx_colour_grade(`neutral.lut.png`)
gfx_option('POST_PROCESSING', false)

gfx_hud_class_add(`Rect`, {
})

obj = gfx_hud_object_add(`Rect`)
obj.position = vec(200, 100)  -- centre of object, from screen bottom left
obj.size = vec(50, 50)
obj.colour = vec(0, 1, 1)
obj.zOrder = 2
obj.alpha = 0.7

obj2 = gfx_hud_object_add(`Rect`)
obj2.position = vec(100, 100)
obj2.size = vec(50, 50)
obj2.colour = vec(1, 1, 0)
obj2.zOrder = 2
obj2.alpha = 0.7
obj2.orientation = 30
obj2.parent = obj

obj2b = gfx_hud_object_add(`Rect`)
obj2b.position = vec(200, 50)
obj2b.size = vec(50, 50)
obj2b.colour = vec(1, 1, 1)
obj2b.zOrder = 2
obj2b.alpha = 0.7
obj2b.orientation = 60
obj2b.parent = obj

obj3 = gfx_hud_object_add(`Rect`)
obj3.position = vec(0, 50)
obj3.size = vec(50, 50)
obj3.colour = vec(1, 0, 0)
obj3.zOrder = 2
obj3.alpha = 0.7
obj3.orientation = 0
obj3.parent = obj2


gfx_render(0.1, vec(0, 0, 0), quat(1, 0, 0, 0))
gfx_screenshot('output-simple.png')
