global nodeArray  -- used to store the nodes (meshes, primitives) created
global colStream

fn min a b = ( if a > b then b else a )

fn dft a b = ( if a == undefined then b else a )

global ini_file_name = "grit.ini"

-- {{{ import stuff

fn readTrimeshVerts &vArray = ( -- {{{
    local inStr
        
    inStr = readLine colStream
    lineArgs = filterString (inStr as string) "; "
    while (lineArgs[1] != "}") do (
        append vArray [lineArgs[1] as float, lineArgs[2] as float, lineArgs[3] as float] 
        inStr = readLine colStream
        lineArgs = filterString (inStr as string) "; "
    ) -- end while
) -- }}}
    
fn readTrimeshfaces &fArray = ( -- {{{
    local inStr
        
    inStr = readLine colStream
    lineArgs = filterString (inStr as string) "; "
    while (lineArgs[1] != "}") do (
        append fArray [(lineArgs[1] as integer)+1, (lineArgs[2] as integer)+1, (lineArgs[3] as integer)+1] 
        inStr = readLine colStream
        lineArgs = filterString (inStr as string) "; "
    ) -- end while
) -- }}}
    
fn readTrimesh = ( -- {{{
    local inStr, vertArray, faceArray

    inStr = readLine colStream -- "  vertexes {"
    
    vertArray = #(); 
    readTriMeshVerts &VertArray
            
    inStr = readLine colStream -- " faces {"

    faceArray = #(); 
    readTriMeshFaces &faceArray

    inStr = readLine colStream -- "}"

    newMesh = mesh vertices:vertArray faces:faceArray
    newMesh.Name = uniquename "Trimesh"
    newMesh -- return value
) -- }}}

fn readHullVerts &vArray= ( -- {{{
    local inStr
        
    inStr = readLine colStream
    lineArgs = filterString (inStr as string) "; "
    while (lineArgs[1] != "}") do (
        append vArray [lineArgs[1] as float, lineArgs[2] as float, lineArgs[3] as float] 
        inStr = readLine colStream
        lineArgs = filterString (inStr as string) "; "
    ) -- end while
) -- }}}
    
fn readHull = ( -- {{{
    local inStr, vertArray, faceArray

    inStr = readLine colStream -- "  vertexes {"
    
    vertArray = #(); 
    readHullVerts &VertArray
            

    inStr = readLine colStream -- "}"

    newMesh = mesh vertices:vertArray --faces:faceArray
    newMesh.Name = uniquename "Hull"
    newMesh -- return value
) -- }}}

-- }}}

    
fn get_phys_mat obj id suppress:false = ( -- {{{
    local mat = obj.material
    if mat == undefined then (
        if not suppress then
            messageBox (obj.name+" has no material:  Assigning frictionless material!\n") title:"ERROR!"
        return undefined
    )

    if (classof mat) == MULTIMATERIAL then (
        mat = mat.materialList[mat.materialIDlist[id]]
    )

    if (classof mat) == GRIT_PHYSMAT then (
        mat
    ) else (
        if not suppress then
            messageBox (obj.name+" has no recognisable material (Grit Physmat or multimaterial of physmats):  Assigning frictionless material!\n") title:"ERROR!"
        undefined
    )
) -- }}}

fn is_obj_convex obj = ( -- {{{
    local mat = obj.material
    if mat == undefined then return false
    if (classof mat) == MULTIMATERIAL then return false
    if (classof mat) != GRIT_PHYSMAT then return false
    mat.convex
) -- }}}

fn get_mat_margin mat = ( -- {{{
    if mat == undefined then return 0.04
    if (classof mat) == MULTIMATERIAL then return 0.04
    if (classof mat) != GRIT_PHYSMAT then return 0.04
    mat.margin
) -- }}}

fn get_mat_hex mat = ( -- {{{
    if mat == undefined then return "0x0"
    if (classof mat) == MULTIMATERIAL then return "0x0"
    if (classof mat) != GRIT_PHYSMAT then return "0x0"
    return formattedPrint mat.physmat format:"#x"
) -- }}}

fn grit_tcol_export out_file propobj objs = ( -- {{{

    format "Creating file: %\n" out_file to:listener
    local savetcol = createFile (out_file)

    local dummy = if propobj==objs[1] then undefined else propobj
        
    format "TCOL1.0\n\n" to:savetcol

    --attributes
    format "attributes {\n" to:savetcol
    format "Started TCOL export\nTCOL 1.0\nName: %\nStarted exporting attributes...\n" savetcol to:listener
    if getUserProp propobj "gritDynamic" == true then
        format "    mass %;\n" (dft (getUserProp propobj "gritMassValue") 100) to:savetcol
    else
        format "    static;\n" to:savetcol
    if getUserProp propobj "gritInertiaExported"==true then
        format "    inertia % % %;\n" (dft (getUserProp propobj "gritInertiaXValue") 100) (dft (getUserProp propobj "gritInertiaYValue") 100) (dft (getUserProp propobj "gritInertiaZValue") 100) to:savetcol
    if getUserProp propobj "gritLinearDampingExported"==true and getUserProp propobj "gritLinearDampingValue" != undefined then
        format "    linear_damping %;\n" (getUserProp propobj "gritLinearDampingValue") to:savetcol
    if getUserProp propobj "gritAngularDampingExported"==true and getUserProp propobj "gritAngularDampingValue" != undefined then
        format "    angular_damping %;\n" (getUserProp propobj "gritAngularDampingValue") to:savetcol
    if getUserProp propobj "gritLinearSleepThresholdExported"==true and getUserProp propobj "gritLinearSleepThresholdValue" != undefined then
        format "    linear_sleep_threshold %;\n" (getUserProp propobj "gritLinearSleepThresholdValue") to:savetcol
    if getUserProp propobj "gritAngularSleepThresholdExported"==true and getUserProp propobj "gritAngularSleepThresholdValue" != undefined then
        format "    angular_sleep_threshold %;\n" (getUserProp propobj "gritAngularSleepThresholdValue") to:savetcol
    if getUserProp propobj "gritCCDMotionThresholdExported"==true and getUserProp propobj "gritCCDMotionThresholdValue" != undefined then
        format "    ccd_motion_threshold %;\n" (getUserProp propobj "gritCCDMotionThresholdValue") to:savetcol
    if getUserProp propobj "gritCCDSweptSphereRadiusExported"==true and getUserProp propobj "gritCCDSweptSphereRadiusValue" != undefined then
        format "    ccd_swept_sphere_radius %;\n" (getUserProp propobj "gritCCDSweptSphereRadiusValue") to:savetcol
    format "}\n\n" to:savetcol
    format "Finished exporting attributes\n" to:listener
    
    --compounds
    format "compound {\n" to:savetcol
    format "Started exporting compounds...\n" to:listener
    local num_trimeshes = 0
    for obj in objs do (
        local sca
        local pos
        local q
        if dummy == undefined then (
            sca = obj.scale
            pos = obj.pos
            q = obj.rotation
        ) else (
            sca = in coordsys dummy obj.scale
            pos = in coordsys dummy obj.pos
            q = in coordsys dummy obj.rotation
        )
        if classof(obj) == Box then (
            local dim = sca * point3 obj.width obj.length obj.height
            local mat = get_phys_mat obj 1
            local margin = min (get_mat_margin mat) ((min(min dim.x dim.y) dim.z)/4)
            pos = pos + point3 0 0 (dim.z/2) * inverse q
            format "    box {\n" to:savetcol
            format "        material %;\n" (get_mat_hex mat) to:savetcol
            format "        margin %;\n" margin to:savetcol
            format "        centre % % %;\n" pos.x pos.y pos.z to:savetcol
            format "        dimensions % % %;\n" dim.x dim.y dim.z to:savetcol
            format "        orientation % % % %;\n" q.w q.x q.y q.z to:savetcol
            format "    }\n" to:savetcol
        )
        if classof(obj) == Cylinder then (
            local dim = sca * point3 (obj.radius*2) (obj.radius*2) obj.height
            local mat = get_phys_mat obj 1
            local margin = min (get_mat_margin mat) ((min(min dim.x dim.y) dim.z)/4)
            pos = pos + point3 0 0 (dim.z/2) * inverse q
            format "    cylinder {\n" to:savetcol
            format "        material %;\n" (get_mat_hex mat) to:savetcol
            format "        margin %;\n" margin to:savetcol
            format "        centre % % %;\n" pos.x pos.y pos.z to:savetcol
            format "        dimensions % % %;\n" dim.x dim.y dim.z to:savetcol
            format "        orientation % % % %;\n" q.w q.x q.y q.z to:savetcol
            format "    }\n" to:savetcol
        )
        if classof(obj) == Cone then (
            -- take average of scale, we cannot actually compress the cone in grit
            local r = (sca.x+sca.y)/2 * obj.radius1
            local h = sca.z * obj.height
            local mat = get_phys_mat obj 1
            local margin = get_mat_margin mat
            pos = pos + point3 0 0 (h/2) * inverse q
            format "    cone {\n" to:savetcol
            format "        material %;\n" (get_mat_hex mat) to:savetcol
            format "        margin %;\n" margin to:savetcol
            format "        centre % % %;\n" pos.x pos.y pos.z to:savetcol
            format "        radius %;\n" r to:savetcol
            format "        height %;\n" h to:savetcol
            format "        orientation % % % %;\n" q.w q.x q.y q.z to:savetcol
            format "    }\n" to:savetcol
        )
        if classof(obj) == Sphere then (
            -- take average of scale, we cannot actually compress the cone in grit
            local r = (sca.x+sca.y+sca.z)/3 * obj.radius
            local mat = get_phys_mat obj 1
            format "    cone {\n" to:savetcol
            format "        material %;\n" (get_mat_hex mat) to:savetcol
            format "        centre % % %;\n" pos.x pos.y pos.z to:savetcol
            format "        radius %;\n" r to:savetcol
            format "    }\n" to:savetcol
        )
        if classof obj.baseobject == Editable_Poly then (
            if not is_obj_convex obj then (
                num_trimeshes = num_trimeshes + 1
                continue
            )
            local mat = get_phys_mat obj 1
            local margin = get_mat_margin mat
            local tmesh
            if dummy == undefined then (
                tmesh = snapshotAsMesh obj
            ) else (
                local old_pos = dummy.pos
                local old_q = dummy.rotation
                local old_sca = dummy.scale
                dummy.pos = [0,0,0]
                dummy.rotation = Quat 0 0 0 1
                dummy.scale = [1,1,1]
                tmesh = snapshotAsMesh obj
                dummy.rotation = old_q
                dummy.scale = old_sca
                dummy.pos = old_pos
            )
            local num_verts = tmesh.numverts 
            format "Verts: %\n" num_verts to:listener
            format "    hull {\n" to:savetcol
            format "        material %;\n" (get_mat_hex mat) to:savetcol
            format "        margin %;\n" margin to:savetcol
            format "        vertexes {\n" to:savetcol
            for v = 1 to num_verts do (
                local vert = getVert tmesh v
                format "            % % %;\n" vert.x vert.y vert.z to:savetcol
            )
            format "        }\n" to:savetcol
            delete tmesh
            format "    }\n" to:savetcol
        )
    )
    format "Finished exporting compounds\n" to:listener
    format "}\n" to:savetcol
    
    --trimesh
    if num_trimeshes > 0 then (
        format "trimesh {\n" to:savetcol
        format "    vertexes {\n" to:savetcol
        format "Started exporting trimesh...\n" to:listener
        for obj in objs do (
            if classof obj.baseobject == Editable_Poly then (
                if is_obj_convex obj then continue
                local tmesh
                if dummy == undefined then (
                    tmesh = snapshotAsMesh obj
                ) else (
                    local old_pos = dummy.pos
                    local old_q = dummy.rotation
                    local old_sca = dummy.scale
                    dummy.pos = [0,0,0]
                    dummy.rotation = Quat 0 0 0 1
                    dummy.scale = [1,1,1]
                    tmesh = snapshotAsMesh obj
                    dummy.rotation = old_q
                    dummy.scale = old_sca
                    dummy.pos = old_pos
                )

                for v = 1 to tmesh.numverts do (
                    local vert = getVert tmesh v
                    format "        % % %;\n" vert.x vert.y vert.z to:savetcol
                )

                delete tmesh
            )
        )
        format "    }\n" to:savetcol
        format "verts exporting finished!\n" to:listener

        format "    faces {\n" to:savetcol
        local initial_vertex_offset = 0
        for obj in objs do (
            if classof obj.baseobject == Editable_Poly then (
                if is_obj_convex obj then continue
                local tmesh = snapshotAsMesh obj

                local errors = 0
                for f = 1 to tmesh.numfaces do (
                    -- tcol indexes are zero-based so subtract 1
                    local face = (getFace tmesh f) - 1 + initial_vertex_offset
                    local mat = get_phys_mat obj (getFaceMatId tmesh f) suppress:(errors>0)
                    if mat == undefined then
                        errors = errors + 1
                    -- 'as integer' suppresses the trailing .0 on the output numbers
                    -- (as that would not be correct tcol syntax)
                    format "        % % % %;\n" (face[1] as integer) (face[2] as integer) (face[3] as integer) (get_mat_hex mat) to:savetcol
                )
                initial_vertex_offset = initial_vertex_offset + tmesh.numverts
                if errors > 0 then (
                    local errmsg = stringstream ""
                    format "There were % errors in \"%\"" errors obj.name to:errmsg
                    messageBox (errmsg AS string) title:"ERROR!"
                )
                delete tmesh
            )
        )
        format "    }\n" to:savetcol
        format "faces exporting finished!\n" to:listener
        format "}\n" to:savetcol
        format "Finished exporting trimesh...\n" to:listener
    )
    format "finished!\n" to:listener
    close savetcol
) -- }}}

global grit_current_tcol_floater = undefined

global GritRolloutTcolMain


Rollout GritRolloutTcolMain "Main" -- {{{
(
    group "Import/Export" (
        button chooseDir "Browse Dir" toolTip:"Choose dir where files are written" across:2
        --button imp "Import tcol"
        button exp "Export Selection" toolTip:"Write TCOL file"
        edittext editDir ""
    )

    -- attributes {{{
    group "Attributes (select a single object to modify)" (
        checkbox dynamic "Dynamic"
        spinner smass "Mass (kg):"
        checkbox cyinertia "Export"
        spinner sinertiax "Inertia Tensor X (m):"
        spinner sinertiay "Inertia Tensor Y (m):"
        spinner sinertiaz "Inertia Tensor Z (m):"
        checkbox cyldamp "Export" across:2 
        spinner sldamp "Linear Damping:"
        checkbox cyadamp "Export" across:2 
        spinner sadamp "Angular Damping:"
        checkbox cylsthrs "Export" across:2 
        spinner slsthrs "Lin. sleep thr. (m/s):"
        checkbox cyasthrs "Export" across:2 
        spinner sasthrs "Ang. sleep thr. (rad/s):"
        checkbox cycmotthrs "Export" across:2
        spinner scmotthrs "CCD motion thr. (m/s):"
        checkbox cycsphswrad "Export" across:2
        spinner scsphswrad "CCD sphere rad (m):"
    )


    fn update = (
        -- we might get called when the floater does not exist
        if grit_current_tcol_floater == undefined then return()

        if (getCurrentSelection()).count != 1 then (
            smass.enabled = false 
            sinertiax.enabled = false 
            sinertiay.enabled = false 
            sinertiaz.enabled = false 
            sldamp.enabled = false 
            sadamp.enabled = false 
            slsthrs.enabled = false 
            sasthrs.enabled = false 
            scmotthrs.enabled = false 
            scsphswrad.enabled = false 

            dynamic.triState = 0
            cyinertia.triState = 0
            cyldamp.triState = 0
            cyadamp.triState = 0
            cylsthrs.triState = 0
            cyasthrs.triState = 0
            cycmotthrs.triState = 0
            cycsphswrad.triState = 0

            dynamic.enabled = false
            cyinertia.enabled = false
            cyldamp.enabled = false
            cyadamp.enabled = false
            cylsthrs.enabled = false
            cyasthrs.enabled = false
            cycmotthrs.enabled = false
            cycsphswrad.enabled = false
        ) else (
            local obj = (getCurrentSelection())[1]

            smass.value = dft (getUserProp obj "gritMassValue") 100
            sinertiax.value = dft (getUserProp obj "gritInertiaXValue") 0
            sinertiay.value =  dft (getUserProp obj "gritInertiaYValue") 0
            sinertiaz.value =  dft (getUserProp obj "gritInertiaZValue") 0
            sldamp.value = dft (getUserProp obj "gritLinearDampingValue") 0.5
            sadamp.value = dft (getUserProp obj "gritAngularDampingValue") 0.5
            slsthrs.value = dft (getUserProp obj "gritLinearSleepThresholdValue") 1.0
            sasthrs.value = dft (getUserProp obj "gritAngularSleepThresholdValue") 0.8
            scmotthrs.value = dft (getUserProp obj "gritCCDMotionThresholdValue") 0
            scsphswrad.value = dft (getUserProp obj "gritCCDSweptSphereRadiusValue") 0
            
            local dyn = dft (getUserProp obj "gritDynamic") false
            dynamic.triState = if dyn then 1 else 0
            cyinertia.triState = if dft (getUserProp obj "gritInertiaExported") false then 1 else 0
            cyldamp.triState = if dft (getUserProp obj "gritLinearDampingExported") false then 1 else 0
            cyadamp.triState = if dft (getUserProp obj "gritAngularDampingExported") false then 1 else 0
            cylsthrs.triState = if dft (getUserProp obj "gritLinearSleepThresholdExported") false then 1 else 0
            cyasthrs.triState = if dft (getUserProp obj "gritAngularSleepThresholdExported") false then 1 else 0
            cycmotthrs.triState = if dft (getUserProp obj "gritCCDMotionThresholdExported") false then 1 else 0
            cycsphswrad.triState = if dft (getUserProp obj "gritCCDSweptSphereRadiusExported") false then 1 else 0

            smass.enabled = dynamic.triState == 1
            sinertiax.enabled = cyinertia.triState == 1
            sinertiay.enabled = cyinertia.triState == 1
            sinertiaz.enabled = cyinertia.triState == 1
            sldamp.enabled = cyldamp.triState == 1
            sadamp.enabled = cyadamp.triState == 1
            slsthrs.enabled = cylsthrs.triState == 1
            sasthrs.enabled = cyasthrs.triState == 1
            scmotthrs.enabled = cycmotthrs.triState == 1
            scsphswrad.enabled = cycsphswrad.triState == 1


            dynamic.enabled = true
            cyinertia.enabled = dyn
            cyldamp.enabled = dyn
            cyadamp.enabled = dyn
            cylsthrs.enabled = dyn
            cyasthrs.enabled = dyn
            cycmotthrs.enabled = dyn
            cycsphswrad.enabled = dyn
        )
    )

    --This is about enabling or disabling the spinners if 'export' is selected
    fn update_from_value name v = (
        setUserProp ((getCurrentSelection())[1]) name v
        update()
    )
    fn update_from_tristate name v = (
        update_from_value name (v.triState==1)
    )
    on dynamic changed v do update_from_value "gritDynamic" v
    on cyinertia changed v do update_from_value "gritInertiaExported" v
    on cyldamp changed v do update_from_value "gritLinearDampingExported" v
    on cyadamp changed v do update_from_value "gritAngularDampingExported" v
    on cylsthrs changed v do update_from_value "gritLinearSleepThresholdExported" v
    on cyasthrs changed v do update_from_value "gritAngularSleepThresholdExported" v
    on cycmotthrs changed v do update_from_value "gritCCDMotionThresholdExported" v
    on cycsphswrad changed v do update_from_value "gritCCDSweptSphereRadiusExported" v

    on smass changed v do update_from_value "gritMassValue" v
    on sinertiax changed v do update_from_value "gritInertiaXValue" v
    on sinertiay changed v do update_from_value "gritInertiaYValue" v
    on sinertiaz changed v do update_from_value "gritInertiZXValue" v
    on sldamp changed v do update_from_value "gritLinearDampingValue" v
    on sadamp changed v do update_from_value "gritAngularDampingValue" v
    on slsthrs changed v do update_from_value "gritLinearSleepThresholdValue" v
    on sasthrs changed v do update_from_value "gritAngularSleepThresholdValue" v
    on scmotthrs changed v do update_from_value "gritCCDMotionThresholdValue" v
    on scsphswrad changed v do update_from_value "gritCCDSweptSphereRadiusValue" v

    -- }}}

    on GritRolloutTcolMain open do
    (
        local lastDir = getINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastDir"
        if lastDir != undefined then (
            editDir.text = lastDir
            setINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastDir" editDir.text	
        )
    )
    on chooseDir pressed do
    (           
        local filename = getSavePath caption:"Output directory" initialDir: editDir.text
        if (filename != undefined) then editDir.text = filename
    )                   

    -- {{{ Exporting
    on exp pressed do (

        if (editDir.text == "") then (
            messageBox "You have to choose an output directory."
            return()
        ) else if (getCurrentSelection()).count == 0 then (
            messageBox "You have to select something for export."
            return()
        )

        for obj in getCurrentSelection() do (
            local out_file = editDir.text + "\\" + obj.name + ".tcol"
            if obj.children.count == 0 then (
                local children = #(obj)
                grit_tcol_export out_file obj children
            ) else (
                grit_tcol_export out_file obj obj.children
            )
        )
    )
    -- }}}
            
    -- {{{ Importing
    on imp pressed do
    (
        colopenname = getOpenFileName caption:"Open TCOL" types:"Open Grit collision file (*.tcol)|*.tcol|"
        colStream = undefined
        if colopenname != undefined then
            colStream = openfile colopenname
        if colStream != undefined do (
            lc = 1
            while not eof colStream do (
                --format "LC = %\n" lc; lc += 1
                inputline = readLine colStream --We start reading the file
                lineArgs = filterString inputline "; "
                
                if lineArgs.count == 0 then
                    continue
                
                if lineArgs[1] == "/*" then (
                    while (lineArgs[1] != "*/") do ( )
                    continue
                )
            
                if lineArgs[1] == "TCOL1.0" then (
                    format "\nStarted TCOL import\nTCOL 1.0\nName: %\n" colopenname to:listener
                    continue
                )
                
                if lineArgs[1] == "//" then (
                    commnt = lineArgs[2] 
                    format "Comment: %\n" commnt to:listener
                    continue
                ) 
                
                if lineArgs[1] == "attributes" then (
                    format "Importing atributes\n" to:listener
                    continue
                ) 
                
                if lineargs[1] == "mass" then (
                    cymass.triState = 1
                    massv = (lineArgs[2] as float) 
                    smass.value = massv
                    format "Mass: %\n" (lineArgs[2] as float)  to:listener
                    continue
                )
                
                if lineargs[1] == "static" then 
                (
                    cymass.triState = 0
                    format "Static\n" to:listener
                    continue
                )
                
                if lineargs[1] == "inertia" then
                (
                    cyinertia.triState = 1
                    sinertiax.value = (lineArgs[2] as float)
                    sinertiay.value = (lineArgs[3] as float)
                    sinertiaz.value = (lineArgs[4] as float)
                    continue
                )

                if lineargs[1] == "linear_damping" then (
                    cyldamp.triState = 1
                    ldampv = (lineArgs[2] as float) 
                    sldamp.value = ldampv
                    format "Linear Damping: %\n" (lineArgs[2] as integer) to:listener
                    continue
                )
                
                if lineargs[1] == "angular_damping" then (
                    cyadamp.triState = 1
                    adampv = (lineArgs[2] as float) 
                    sadamp.value = adampv
                    format "Angular Damping: %\n" (lineArgs[2] as integer) to:listener
                    continue
                )
                
                if lineargs[1] == "linear_sleep_threshold" then (
                    cylsthrs.triState = 1
                    lsthrsv = (lineArgs[2] as float) 
                    slsthrs.value = lsthrsv
                    format "Linear sleep threshold: %\n" (lineArgs[2] as integer) to:listener
                    continue
                )
                
                if lineargs[1] == "angular_sleep_threshold" then (
                    cyasthrs.triState = 1
                    asthrsv = (lineArgs[2] as float) 
                    sasthrs.value = asthrsv
                    format "Angular sleep threshold: %\n" (lineArgs[2] as integer) to:listener
                    continue
                )
                
                if lineargs[1] == "ccd_motion_threshold" then (
                    cycmotthrs.triState = 1
                    cmotthrsv = (lineArgs[2] as float) 
                    scmotthrs.value = cmotthrsv
                    format "CCD Motion Threshold: %\n" (lineArgs[2] as integer) to:listener
                    continue
                )
                
                if lineargs[1] == "ccd_swept_sphere_radius" then (
                    cycsphswrad.triState = 1
                    csphswradv = (lineArgs[2] as float) 
                    scsphswrad.value = csphswradv
                    format "CCD Swept Sphere Radius: %\n" (lineArgs[2] as integer) to:listener
                    continue
                )
 
                if lineArgs[1] == "}" do (
                    format "Finished \n" to:listener 
                    continue
                )
                
                -- Trimesh
                
                if lineArgs[1] == "trimesh" then (
                    format "Importing trimesh\n" to:listener
                    readTrimesh()
                    continue
                ) 
                
                if lineArgs[1] == "hull" then (
                    format "Ignoring hull\n" to:listener
                    --readHull()
                    continue
                ) 
                
                -- Compounds
                    
                if lineArgs[1] == "compound" then (
                    format "Importing compounds\n" to:listener
                    continue
                ) 
                
                if lineArgs[1] == "box" then (
                    while (lineArgs[1] != "}") do
                    (
                        inStr = readLine colStream
                        lineArgs = filterString (inStr as string) "; "
                        if lineArgs[1] == "material" then (
                            M = lineArgs[2]
                            continue
                        )
                        if lineArgs[1] == "centre" then (
                            X = lineArgs[2] as float
                            Y = lineArgs[3] as float
                            ZG = lineArgs[4] as float
                            continue
                        )
                        if lineArgs[1] == "dimensions" then (
                            L = lineArgs[2] as float
                            W = lineArgs[3] as float
                            H = lineArgs[4] as float
                            continue
                        )
                        Z=ZG-(H/2)
                        b = box length:L width:W height:H pos:[X,Y,Z]
                        format "Imported % at % % % length:% width:% height:% material:%\n" b.name X Y Z b.width b.length b.height M to:listener
                    ) -- end while
                    continue
                )
                
                if lineArgs[1] == "cylinder" then (
                    while (lineArgs[1] != "}") do (
                        inStr = readLine colStream
                        lineArgs = filterString (inStr as string) "; "
                        if lineArgs[1] == "material" then (
                            M = lineArgs[2]
                            continue
                        )
                        if lineArgs[1] == "centre" then (
                            X = lineArgs[2] as float
                            Y = lineArgs[3] as float
                            ZG = lineArgs[4] as float
                            continue
                        )
                        if lineArgs[1] == "dimensions" then (
                            RX = lineArgs[2] as float
                            RY = lineArgs[3] as float
                            H = lineArgs[4] as float
                            continue
                        )
                        Z=ZG-H/2
                        c = cylinder radius:(RX) height:H pos:[X,Y,Z]
                        scale c [1,RY/RX,1]
                        format "Imported % at % % % Dimension X:% Dimension Y:% height:%\n material:%\n" c.name X Y Z (RX/2) RY c.height M to:listener
                    ) -- end while
                    continue
                )
                
                if lineArgs[1] == "cone" then (
                    while (lineArgs[1] != "}") do (
                        inStr = readLine colStream
                        lineArgs = filterString (inStr as string) "; "
                        if lineArgs[1] == "material" then (
                            M = lineArgs[2] as float
                            continue
                        )
                        if lineArgs[1] == "centre" then (
                            X = lineArgs[2] as float
                            Y = lineArgs[3] as float
                            ZG = lineArgs[4] as float
                            continue
                        )
                        if lineArgs[1] == "radius" then (
                            R = lineArgs[2] as float
                            continue
                        )
                        if lineArgs[1] == "height" then (
                            H = lineArgs[2] as float
                            continue
                        )
                        Z=ZG-H/2
                        c = cone radius1:R radius2:0 height:H pos:[X,Y,Z]
                        format "Imported % at % % % radius:% height:% material:%\n" c.name X Y Z c.radius1 c.height M to:listener
                    ) -- end while
                    continue
                )

                if lineArgs[1] == "sphere" then (
                    while (lineArgs[1] != "}") do (
                        inStr = readLine colStream
                        lineArgs = filterString (inStr as string) "; "
                        if lineArgs[1] == "material" then
                        (
                            M = lineArgs[2] as float
                            continue
                        )
                        if lineArgs[1] == "centre" then
                        (
                            X = lineArgs[2] as float
                            Y = lineArgs[3] as float
                            Z = lineArgs[4] as float
                            continue
                        )
                        if lineArgs[1] == "radius" then
                        (
                            R = lineArgs[2] as float
                            continue
                        )
                        s = sphere radius:R pos:[X,Y,Z]
                        format "Imported % at % radius:% material:%\n" s.name s.pos s.radius M to:listener
                    ) -- end while
                    continue
                )                    
            ) -- this ends the While loop
            
            close colStream
        ) -- this ends the "if colstream" test
        
    ) -- this ends the function }}}
    
) -- }}}



Rollout GritRolloutTcolHelp "Help" -- {{{
(
    group "Help" (
        label lab1 "*.tcol Script by JostVice for Grit"
        label bspc " "
        label lab2 "Usage: "
        label lab3 "Export and import your phys meshes compounds and trimeshes"
        label lab4 "with his selected attributes."
        label lab5 "Use the Grit Phys Mat material to set up the materials."
        label lab6 "If a physmat isn't found, it will default to Frictionless."
        label lab7 "Don't let this happen!"
        label bspc2 " "
    )
    group "Known bugs" (
        label lab8 "Importing is very buggy. Trimeshes with attributes"
        label lab9 "like margin will output a error. simply edit your tcol"
        label lab10 "and delete those lines. Hull and Plane isn't supported."
        label bspc3 " "
        label lab11 "Cylinder is weird, you can probably do a cylinder"
        label lab12 "manually by using a box and copying by hand coords."
    )
) -- }}}



plugin Material GRIT_PHYSMAT name:"GRIT Phys Mat" classID:#(0x25d32aa6, 0x2d8ae265) extends:Standard replaceUI:true  -- {{{
(
    local SurfAry = #( "Frictionless", "Rubber", "Metal", "Plastic",
                       "PolishedWood", "PolishedStone", "Wood", "Stone",
                       "Mud", "Gravel", "Sand" )

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
    
) -- }}}


callbacks.removescripts id:#GritTcolSelection
callbacks.addScript #selectionSetChanged "GritRolloutTcolMain.update()" id:#GritTcolSelection

macroScript showGritTCOLExporter
    category:"Grit Tools"
    internalCategory:"Grit Tools"
    buttonText:"Grit .tcol Exporter"
    tooltip:"Grit .tcol Exporter"
    Icon:#("Maintoolbar",49)
(
    if grit_current_tcol_floater != undefined then (
        closeRolloutFloater grit_current_tcol_floater
    )
    grit_current_tcol_floater = newRolloutFloater "Grit .tcol Exporter" 320 500

    addRollout GritRolloutTcolMain grit_current_tcol_floater
    addRollout GritRolloutTcolHelp grit_current_tcol_floater

    GritRolloutTcolMain.update()
)


