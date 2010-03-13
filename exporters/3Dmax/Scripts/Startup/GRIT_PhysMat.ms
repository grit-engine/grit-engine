plugin Material GRIT_PHYSMAT
    name:"GRIT Phys Mat"
    classID:#(0x25d32aa6, 0x2d8ae265)
    extends:Standard replaceUI:true 
(
    local SurfAry = #( "Frictionless", "Rubber", "Metal",
                       "Plastic", "PolishedWood", "PolishedStone", "Wood",
                       "Stone", "Mud", "Gravel", "Sand" )

    parameters main rollout:params
    (
        physmat type:#integer default:0 ui:pmat
        physidmat type:#string default:"Default" ui:pmatnam
        convex type:#boolean default:false ui:pconv
        margin type:#float default:0.04 ui:pmargin

        on physmat set val do (
            if val == 0 then delegate.diffuse = color 255 20 147
            else if val == 1 then delegate.diffuse = color 190 190 190
            else if val == 2 then delegate.diffuse = color 217 173 245
            else if val == 3 then delegate.diffuse = color 148 0 0
            else if val == 4 then delegate.diffuse = color 255 165 10
            else if val == 5 then delegate.diffuse = color 255 230 224
            else if val == 6 then delegate.diffuse = color 140 72 43
            else if val == 7 then delegate.diffuse = color 209 209 209
            else if val == 8 then delegate.diffuse = color 102 50 11
            else if val == 9 then delegate.diffuse = color 213 221 214
            else if val == 10 then delegate.diffuse = color 255 233 194
            physidmat = SurfAry[(val+1)]
            delegate.bounce = 3
        )
    )
    
    rollout params "Physical material attributes"
    (
        spinner pmat "Material: " fieldwidth:45 type:#integer align:#center range:[0,10,0] across:2
        edittext pmatnam "" align:#center enabled:false 
        checkbox pconv "Polys as convex hull" align:#center across:2
        spinner pmargin "Default margin: " fieldwidth:45 type:#float align:#center range:[0,1,0.04]
        
    )--end rollout

    on create do (
        delegate.adLock = on
        delegate.specular = color 15 15 15
    )
    
)

include "tcolscript.ms"

macroScript showGritTCOLExporter
    category:"Grit Tools"
    internalCategory:"Grit Tools"
    buttonText:"Grit TCOL Exporter"
    tooltip:"Grit Exporter"
    Icon:#("Maintoolbar",49)
(
    local menu = newRolloutFloater "Grit TCOL exporter" 320 500

    addRollout grit_tcol_import_export menu
    addRollout grit_tcol_help menu

    grit_tcol_import_export.resetb.pressed()
)

