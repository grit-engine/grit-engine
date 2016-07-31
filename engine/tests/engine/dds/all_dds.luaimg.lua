function gen_mipmaps(img)
    local tab = {}
    local counter = 1
    local w = img.width
    local h = img.height
    while w>1 or h>1 do
        tab[counter] = img:scale(vec(w, h), "BOX")
        w = w == 1 and 1 or ceil(w/2)
        h = h == 1 and 1 or ceil(h/2)
        counter = counter + 1
    end
    return tab
end

function output(fmt, img)
    dds_save_simple("all_dds."..fmt..".dds", fmt, gen_mipmaps(img))
end

img1 = make(vec(128, 128), 1, function(p) return p.x/128 + p.y/128 / 2 end)
img1a = make(vec(128, 128), 1, true, function(p) return vec(p.x/128, p.y/128) end)
img2 = make(vec(128, 128), 2, function(p) return p / 128 end)
img3 = make(vec(128, 128), 3, function(p) return vec3(p/128, 1 - p.x/128/2) end)
img3a = make(vec(128, 128), 3, true, function(p) return vec4(p/128, 1 - p.x/128/2, 1 - mod(p.y/128 * 2, 1)) end)

output("R5G6B5", img3)
output("R8G8B8", img3)
output("A8R8G8B8", img3a)
output("A2R10G10B10", img3a)
output("A1R5G5B5", img3a)
output("R8", img1)
output("R16", img1)
output("G16R16", img2)
output("A8R8", img1a)
output("A4R4", img1a)
output("A16R16", img1a)
output("R3G3B2", img3)
output("A4R4G4B4", img3a)
output("BC1", img3a)
output("BC2", img3a)
output("BC3", img3a)
output("BC4", img1)
output("BC5", img2)

