get_material("base"):load()
get_material("base'"):load()


echo("Loading GTA San Andreas materials")
read_matbin("san_andreas.matbin", function (mname, r,g,b,a, tname, has_alpha, no_cull, lighting, stain)

                                
        has_alpha = has_alpha or stain

        local mat1 = get_material(mname)
        local mat2 = get_material(mname.."'")

        if mat1 ~= nil then remove_material(mat1.name) end
        if mat2 ~= nil then remove_material(mat2.name) end

        mat1 = nil
        mat2 = nil

        local parent = "base"

        mat1 = mat1 or get_material(parent):clone(mname)
        mat2 = mat2 or get_material(parent.."'"):clone(mname.."'")

        for _,mat in ipairs{mat1,mat2} do

                for t=0,mat:getNumTechniques()-1 do

                        if mat:isTechniqueSupported(t) then

                                for p=0,mat:getNumPasses(t)-1 do

                                        if no_cull then
                                                mat:setCull(t,p,false)
                                                mat:setManualCull(t,p,false)
                                        end

                                        if lighting then
                                                mat:setAmbient(t,p,r,g,b)
                                                mat:setDiffuse(t,p,r,g,b,a)
                                        else
                                                mat:setAmbientVertex(t,p,true)
                                                mat:setLightingEnabled(t,p,false)
                                        end
            

                                        if has_alpha or mat==mat2 then
                                                local alpha_reject = stain and 0 or 63
                                                mat:setSceneBlending(t,p,"SRC_ALPHA","ONE_MINUS_SRC_ALPHA")
                                                mat:setTransparentSortingForced(t,p,true)
                                                if mat==mat1 then
                                                        mat:setAlphaRejection(t,p,">",alpha_reject)
                                                else
                                                        -- mat:setDepthBias(t,p,-10,0)
                                                        if mat:hasFragmentProgram(t,p) then
                                                                mat:setFragmentConstantFloat(t,p,"alpha_rej",alpha_reject/255)
                                                        end
                                                end
                                        end

                                        if stain then
                                                mat:setDepthWriteEnabled(t,p,false)
                                        end

                                end

                        end

                end

        end

        mat1:unload()
        mat2:unload()

        for _,mat in ipairs{mat1,mat2} do

                for t=0,mat:getNumTechniques()-1 do

                        if mat:isTechniqueSupported(t) then

                                for p=0,mat:getNumPasses(t)-1 do

                                        if tname then
                                                mat:setTextureName(t,p,0,tname)
                                                mat:setColourOperation(t,p,0,"MODULATE","CURRENT","TEXTURE",0,0,0,0,0,0,0);
                                        else
                                                mat:removeTextureUnitState(t,p,0)
                                        end
                                end

                        end

                end

        end
end)

world:deactivateAll()

-- vim: shiftwidth=8:tabstop=8:expandtab
