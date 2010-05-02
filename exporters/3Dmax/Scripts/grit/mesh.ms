----------------------------------------------------------------------------------------
-- ----------------------------------- WRITE MESH ----------------------------------- --
----------------------------------------------------------------------------------------

-----------------------------------------------------------------
-- Global 'variables'
-- There is an array with vertices datas for each submesh
-- There is also an array made up of faces.
-----------------------------------------------------------------

verticesArrays=#()
facesArrays=#()
boneAssignments=#()
submeshesId=#()

-- verticesArrays[i] will describe the subMesh number i and its elements will be ogreVertices where
--    ogreVert = #(x,y,z,nx,ny,nz,r,g,b,a,u1,v1,u2,v2....)
--                 ----- -------- ------- ---------------
   --                  Pos     Nmal    RGBA       UVs sets
-- RGBA being the  vertex color when relevant     (Optional)
-- A vertex can contain multiple UVs sets        (Optional)

-- facesArrays[i] is an array of Point3 describing the subMesh i. Each Point3 contains vertices indices for a face that is gonna be exported

-- boneAssignments[i] describes the bones assignements for subMesh i. It is an array of triplets like #(vertexIndex,boneIndex,weight)

-- submeshesId is the list of material Ids

-- materialName is the name that Ogre will use




----------------------------------------------------------------
-- returns the correct normal for the vertex
-- according to smoothing groups
-----------------------------------------------------------------
function calc_normal m face faceVertexIndex = -- {{{
(
    local sg = getFaceSmoothGroup m face

    if sg == 0 then (

            -- optimisation: if not smooth use face normal
            getFaceNormal m face

    ) else (

            local v = (getFace m face)[faceVertexIndex]
            local n = [0, 0, 0] -- point3

            for adj_face in (meshop.getFacesUsingVert m v) do (
                -- if we're smooth with this face then add to normal
                if (bit.and sg (getFaceSmoothGroup m adj_face)) != 0 then
                    n += getFaceNormal m adj_face
            )

            normalize n
    )     
) -- }}}

/*
----------------------------------------------------------------------------
-- Retrieves al the datas which wil be used
-- tmesh is the mesh object
-- blened : number of blended textures
-- exportColor = true if you want to export vertices colors
-- sk : the skin modifier (may be undefined)
-- phy : the physique modifier (may be undefined)
-- stores datas in verticesArrays and facesArrays.
----------------------------------------------------------------------------
function getDatas tmesh blended exportAmbient exportAlpha exportHelpers sk phy =
( -- {{{
    
    -- initialisation of the current ogre face as a vector
    local ogreFace=Point3 12 12 12
    
    -- Data arrays init.    
    verticesArrays=#()
    facesArrays=#()
    boneAssignments = #()
    submeshesId = #()

    -- compute bones list for the model.
    BonesList=#()
    computeBonesList phy sk exportHelpers

    local numFaces = (getNumFaces tmesh)

    -- For each face
    for face = 1 to numFaces do         -- LOOP on FACES --
    (        
        GritRolloutOgreMain.exportProgress.value = (100.0*face/numFaces)
    
        local faceVerticesIndices = getFace tmesh face
        local matId = getFaceMatId tmesh face ;        
        
        -- Vertex Color Face
        local faceVertexColorIndices = Point3 1 1 1
        if (exportAmbient) then
            faceVertexColorIndices = meshOp.getMapFace tmesh 0 face ;    -- 0 is the vertex color channel

        local faceVertexAlphaIndices = Point3 1 1 1
        if (exportAlpha) then
            faceVertexAlphaIndices = meshOp.getMapFace tmesh -2 face ;    -- -2 is the vertex alpha channel

        
        -- For each vertex whose face is made up of.
        for localVertexIndex=1 to 3 do                 -- LOOP on VERTICES --
        (            
            local vertexIndex = (int) (faceVerticesIndices[localVertexIndex])
            local vertexColorIndex = (int) (faceVertexColorIndices[localVertexIndex])
            local vertexAlphaIndex = (int) (faceVertexAlphaIndices[localVertexIndex])
            local vertexNormal = calc_normal tmesh face localVertexIndex

            -- we retrieve datas. it depends on options.
            -- UV face
            local UVcoords=#()
            if (exportUV) then
            (
                for ch=1 to numUVsets do
                (
                    local vertInds = meshOp.getMapFace tmesh ch face
                    local UVW = meshOp.getMapVert tmesh ch vertInds[localVertexIndex] ;    -- retrieve the UV for the corresponding channel.
                    append UVcoords UVW[1]
                    append UVcoords UVW[2]
                    --append UVcoords UVW[3]
                )
            )
        
            ogreVertNum = 1

            -- Maybe the material found defines a new submesh...
            if (verticesArrays[matId] == undefined) then
            (
                format "- Submesh detected: material ID = %\n" matId
                verticesArrays[matId] = #()
                boneAssignments[matId] = #()
                append submeshesId matId
            )

            -- the vertex number for the current submesh must be updated
            -- vertex number is increased
            ogreVertNum = verticesArrays[matId].count + 1;
            
            local data = #()
            append data ogreVertNum
            n = copy vertexNormal
            append data n
            if (exportAmbient) then
            (
                append data VertexColorIndex
            )
            if (exportAlpha) then
            (
                append data VertexAlphaIndex
            )
            if (exportUV) then
            (
                if (UVcoords.count > 0) then
                (
                    for uv=1 to UVcoords.count do
                        append data UVcoords[uv]
                )
            )
                
            -- we retrieve data
            local vertexPosition = getVert tmesh faceVerticesIndices[localVertexIndex]
            
            local vertexColor = Point3 0 0 0
            local vertexAlpha = 255
            if (exportAmbient) then
            (
                vertexColor = (meshOp.getMapVert tmesh 0 vertexColorIndex)
            )
            
            if (exportAlpha) then
            (
                vertexAlpha = (meshOp.getMapVert tmesh -2 vertexAlphaIndex)[1]
            )
            
            -- store the vertex in verticesArrays
            vertexData = #(vertexPosition[1],vertexPosition[2],vertexPosition[3],vertexNormal[1],vertexNormal[2],vertexNormal[3],vertexColor[1],vertexColor[2],vertexColor[3],vertexAlpha)
            if (exportUV) then
            (
                for ch=1 to numUVsets do
                (
                    append vertexData UVcoords[1+(ch-1)*2]
                    append vertexData UVcoords[2+(ch-1)*2]
                )
            )
            append verticesArrays[matId] vertexData ;   -- Vertex is added to datas
            
            -- And now, bone assignments. (if a skin modifier is present)
            -- It seems that there are issues when just few vertices have bone assignments.
            -- So there is at least the root assignment.
            if (sk != undefined) then
            (
                local vertexWeightCount = skinOps.getVertexWeightCount sk vertexIndex ;

                if (vertexWeightCount > 4) then
                (
                    format "*** vertex % has more than 4 bones assigned...\n" vertexIndex;    
                )
                
                for k=1 to vertexWeightCount do
                (
                    bid = skinOps.getVertexWeightBoneID sk vertexIndex k
                    bname = skinOps.getBoneName sk bid 1
                    bname = replaceSpaces bname
                    local vertexWeight = skinOps.getVertexWeight sk vertexIndex k
                    
                    id_bone = findItem BonesList bname
                    if (id_bone != 0) then
                        append boneAssignments[matId] #(ogreVertNum-1,id_bone-1,vertexWeight)
                )
                -- assignment to the root bone.
                if (vertexWeightCount==0) then
                (
                    -- gets the root Id:
                    local rootId=getRootsId sk
                    rootname = skinOps.getBoneName sk rootId[1] 1
                    id_bone = findItem BonesList rootname
                    if (id_bone != 0) then
                        append boneAssignments[matId] #(ogreVertNum-1,id_bone-1,1)
                )
            )
            -- same thing with physique modifier if defined
            if (phy != undefined) then
            (
                local vertexWeightCount = physiqueOps.getVertexBoneCount $ vertexIndex
                
                if (vertexWeightCount > 4) then
                (
                    format "*** vertex % has more than 4 bones assigned...\n" vertexIndex;    
                )
                
                for k=1 to vertexWeightCount do
                (
                    bone = physiqueOps.getVertexBone $ vertexIndex k
                    local vertexWeight = physiqueOps.getVertexWeight $ vertexIndex k
                    
                    -- search the bone number
                    bname = replaceSpaces bone.name
                    id_bone = findItem BonesList bname
                    if (id_bone!=0)    then                
                        append boneAssignments[matId] #(ogreVertNum-1,id_bone-1,vertexWeight)
                )
                -- assignment to the first bone if the vertex was not assigned.
                if (vertexWeightCount==0) then
                (
                    -- gets the root Id:
                    append boneAssignments[matId] #(ogreVertNum-1,0,1)
                )
            )

    
            ogreFace[localVertexIndex] = ogreVertNum - 1;        
        )
        
        if (facesArrays[matId] == undefined) then
            facesArrays[matId] = #() ;        
        append facesArrays[matId] #(ogreFace[1],ogreFace[2],ogreFace[3]) ;   -- Face is added to datas        
    )
) -- }}}

-------------------------------------------------
-- writes in the mesh.xml file
-- outName : filename
-- exportUV = true if you want to export texture
-- numUVsets : number of UVs sets per vertex to be exported (only relevant if exportUV = true)
-- exportColor = true if you want to export vertices colors
-- material : material used by the mesh
-- Uses the arrays verticesArrays and facesArrays
-------------------------------------------------
function WriteM blended exportAmbient exportAlpha material outName = -- {{{
(
    local hasSkeleton = false
    local texCoordString = ""
    if (exportUV) then
    (
        texCoordString = "texture_coords=\"" + (numUVsets as string) + "\" "
        for num=1 to numUVsets do
        (
            texCoordString =  texCoordString + "texture_coord_dimensions_" + ((num-1) as string) + "=\"2\" "; -- I really don't care about the W coordinates
        )
    )
    local hasColours = "false" ;    
    if (exportAmbient) then
        hasColours = "true" ;        
    

    -- submeshes start
    format("\t<submeshes>\n") to:outFile
    
    -- for each submesh
    for matId in submeshesId do
    (
        -- Do we need 32-bit indexes?
        use32bitindexes = "false"
        if arrayLength verticesArrays[matId] > 65535 then
            use32bitindexes = "true"
        
        -- get the name of the sub material if needed.
        local submatname = replacespaces material.name
        
        if (classof material == MultiMaterial) then
        (
            if (material.materialList[matId]==undefined) then
            (
                msg = ""
                format "You are using in your mesh a material Id (%) that does not exist in your MultiMaterial (%)\nMaterial information won't be properly exported." matId submatname to:msg
                messageBox msg;    
            )
            else
            (
                submatname  += "/" + replacespaces material.materiallist[matId].name
            )
        )
            
                                    -- HELLO ! --
            ---------------------------------------------------------------------------------
            -- IF YOU COME HERE BECAUSE YOUR OBJECT FAILED EXPORTING, MAYBE THAT'S BECAUSE --
            -- YOU USE MATERIAL ID IN YOUR MESH THAT DOESN'T EXIST  IN YOUR MULTIMATERIAL  --
            ---------------------------------------------------------------------------------
        
        format("\t\t<submesh material = \"%\" usesharedvertices=\"false\" use32bitindexes=\"%\">\n") submatname use32bitindexes to:outFile
    
        format "- writing faces...\n"        
        format("\t\t\t<faces count=\"%\">\n") (arraylength facesArrays[matId]) to:outFile
        
        -- for each face
        for f in facesArrays[matId] do
        (
            format("\t\t\t\t<face ") to:outFile
            format("v1=\"%\" v2=\"%\" v3=\"%\" />\n") ((int)f[1]) ((int)f[2]) ((int)f[3]) to:outFile
        )
        format("\t\t\t</faces>\n") to:outFile

        format "- writing vertices...\n"    
        format("\t\t\t<geometry vertexcount=\"%\">\n") (arrayLength verticesArrays[matId] ) to:outFile
        format("\t\t\t\t<vertexbuffer positions=\"true\" normals=\"true\" colours_diffuse=\"%\" %>\n") hasColours texCoordString to:outFile ;        
        
        -- for each vertex
        for v in verticesArrays[matId] do
        (    
            format("\t\t\t\t\t<vertex>\n") to:outFile
            format("\t\t\t\t\t\t<position x=\"%\" y=\"%\" z=\"%\" />\n") v[1] v[2] v[3] to:outFile
            format("\t\t\t\t\t\t<normal x=\"%\" y=\"%\" z=\"%\" />\n") v[4] v[5] v[6] to:outFile
            if (exportUV) then
            (
                for ch=1 to numUVsets do
                    format("\t\t\t\t\t\t<texcoord u=\"%\" v=\"%\" />\n") v[11+((ch-1)*2)] (1 - v[12+((ch-1)*2)]) to:outFile ;        
            )
            if (exportAmbient) then
            (
                color_string = (v[7] as string) + " " + (v[8] as string) + " " + (v[9] as string) + " " +(v[10] as string)
                format("\t\t\t\t\t\t<colour_diffuse value=\"%\" />\n") color_string to:outFile
            )
            format("\t\t\t\t\t</vertex>\n") to:outFile
        )    
        format("\t\t\t\t</vertexbuffer>\n") to:outFile
        format("\t\t\t</geometry>\n") to:outFile
                    
    
        -- and now bone assignments (and skeleton), if there is at least one element in boneAssignments array.        
        if ((arrayLength boneAssignments[matId]) != 0) then
        (
            hasSkeleton = true
            format "- writing bone assignments...\n"    
            format("\t\t\t<boneassignments>\n") to:outFile
                
            for a in boneAssignments[matId] do
            (
                format("\t\t\t\t<vertexboneassignment vertexindex=\"%\" boneindex=\"%\" weight=\"%\" />\n") a[1] a[2] a[3] to:outFile
            )
            format("\t\t\t</boneassignments>\n") to:outFile
        )
    
        -- submesh end
        format("\t\t</submesh>\n") to:outFile
    )
    
    -- submeshes end
    format("\t</submeshes>\n") to:outFile

    -- Skeleton link if there is at least one bone assignement.    
    if (hasSkeleton) then
    (
        t = filterstring outName "\\"
        format ("\t<skeletonlink name=\"%\"/>\n") (t[arrayLength t] + ".skeleton") to:outFile ;                
    )
                
    
) -- }}}
*/

function get_map_vert mesh channel face fv = (
    meshop.getMapVert mesh channel ((meshop.getMapFace mesh channel face)[fv])
)

function write_mesh_xml selected options out_name = -- {{{
(
    local skin = getSkin selected
    if (skin != undefined) then
    (
        format "Skin modifier detected.\n"
        -- in order to perform, skin should be opened
        max modify mode
        modPanel.setCurrentObject selected.modifiers[#Skin] ;        
    )

    local physique = getPhysique selected
    if (physique != undefined) then
    (
        format "Physique modifier detected.\n"
        -- in order to perform, physique should be opened
        max modify mode
        modPanel.setCurrentObject selected.modifiers[#Physique]
        --physiqueOps.setInitialPose selected true
    )

    local texture = options.blended > 0 and meshop.getMapSupport m 1
    local blend = options.blended > 1
    local num_blend = options.blended - 1
    local num_blend_coords = (int)(ceil (num_blend/4.0))
    local texcoords = (if texture then 1 else 0) + (if blend then 1 else 0) + num_blend_coords

     -- ranges from 1 to 4, all the others are 4
    local last_texcoord_dim = 4 - (num_blend_coords*4 - num_blend)
    
    local cloned
    maxops.clonenodes selected newNodes:&cloned
    local m = cloned[1]
    m.position = [0, 0, 0]
    m.scale = [1, 1, 1]
    m.rotation = Quat 0 0 0 1
    m = convertToMesh m
        
    if options.exportAmbient and not (meshop.getMapSupport m 0) then
        options.exportAmbient = false
    if options.exportAlpha and not (meshop.getMapSupport m -2) then
        options.exportAlpha = false

    format "Calculating normals from smoothing groups...\n "

    -- We're going to have to split vertexes as max won't do this for us.
    -- Consequently we will end up with a 1:many ratio between max vertexes and ogre
    -- vertexes.  The ogre vertexes that represent a given max vertex will each have a different normal, as defined by     -- the needs of a particular face.

    struct OgreVert (pos, normal, ambient, alpha, texcoord, sharpness, weights)

    local num_ogre_vertexes = 0
    local ogre_vert_pos = #()
    local ogre_vert_normal = #()
    local ogre_vert_ambient = #()
    local ogre_vert_alpha = #()
    local ogre_vert_texcoord = #()
    local ogre_vert_sharpness = #()
    local ogre_vert_weights = #()
    for i=1 to num_blend do (
        append ogre_vert_weights #()
    )
    local max_vert_ogre_verts = #() -- a list of ogre vertexes generated for a single max vert

    local face_to_ogre_vert = #()

    local num_max_faces = getNumFaces m
    local mat_ids = #()

    local next_ogre_vertex = 1
    for face = 1 to num_max_faces do (
        GritRolloutOgreMain.exportProgress.value = (50.0*face)/num_max_faces
        local max_indexes = getFace m face
        local mat_id = getFaceMatId m face
        appendIfUnique mat_ids mat_id
        local ogre_face = [1,1,1]
        for fv=1 to 3 do (
            local v = (max_indexes[fv])
            if max_vert_ogre_verts[v] == undefined then max_vert_ogre_verts[v] = #()
            ogre_vert_pos[next_ogre_vertex] = (meshop.getVert m v node:m)
            ogre_vert_normal[next_ogre_vertex] = (calc_normal m face fv)
            ogre_vert_ambient[next_ogre_vertex] = (if options.exportAmbient then get_map_vert m 0 face fv else [1,1,1])
            ogre_vert_alpha[next_ogre_vertex] = (if options.exportAlpha then (get_map_vert m -2 face fv)[1] else 1)
            ogre_vert_texcoord[next_ogre_vertex] = (if texture then get_map_vert m 1 face fv else [0,0])
            ogre_vert_sharpness[next_ogre_vertex] = (if blend then (get_map_vert m 2 face fv)[1] else 0)
            for i=0 to num_blend-1 do (
                local max_map_channel = 3+(int)(floor ((i)/3.0))
                local max_col_channel = (int)(mod i 3) + 1
                local weight = (get_map_vert m max_map_channel face fv)[max_col_channel]
                ogre_vert_weights[i+1][next_ogre_vertex] = weight
            )
            -- search for existing face the same as this new face
            local existing = undefined
            for i=1 to max_vert_ogre_verts[v].count do (
                local ov1 = next_ogre_vertex
                local ov2 = max_vert_ogre_verts[v][i]
                if ogre_vert_normal[ov1] == ogre_vert_normal[ov2] and
                   ogre_vert_ambient[ov1] == ogre_vert_ambient[ov2] and
                   ogre_vert_alpha[ov1] == ogre_vert_alpha[ov2] and
                   ogre_vert_texcoord[ov1] == ogre_vert_texcoord[ov2] and
                   ogre_vert_sharpness[ov1] == ogre_vert_sharpness[ov2] and
                   ogre_vert_weights[ov1] == ogre_vert_weights[ov2] then (
                    for i=1 to num_blend do (
                        if ogre_vert_weights[i][ov1] != ogre_vert_weights[i][ov2] then (
                            continue
                        )
                    )
                    existing = ov2
                )
            )
            if existing == undefined then (
                ogre_face[fv] = next_ogre_vertex
                append (max_vert_ogre_verts[v]) next_ogre_vertex
                next_ogre_vertex = next_ogre_vertex + 1
            ) else (
                ogre_face[fv] = existing
            )
        )
        append face_to_ogre_vert ogre_face
    )
    num_ogre_vertexes = next_ogre_vertex-1


    format "Writing in file %.mesh.xml : \n" out_name
    local out_file = createfile (out_name + ".mesh.xml")

    format "<mesh>\n" to:out_file

    format "    <sharedgeometry>\n" m.numverts to:out_file
    format "        <vertexbuffer " to:out_file
    format "positions=\"true\" " to:out_file
    format "normals=\"true\" " to:out_file
    format "colours_diffuse=\"%\" " (options.exportAmbient or options.exportAlpha) to:out_file
    format "texture_coords=\"%\" " texcoords to:out_file
    if texture then
        format "texture_coord_dimensions_0=\"2\" " to:out_file
    if blend then (
        format "texture_coord_dimensions_1=\"1\" " to:out_file
        for i = 1 to num_blend_coords-1 do (
            format "texture_coord_dimensions_%=\"4\" " (i+1) to:out_file
        )
        format "texture_coord_dimensions_%=\"%\" " (num_blend_coords+1) last_texcoord_dim to:out_file
    )
    format ">\n" to:out_file
    for ov = 1 to num_ogre_vertexes do (
        GritRolloutOgreMain.exportProgress.value = 50.0 + (25.0*ov)/num_ogre_vertexes
        format "            <vertex>\n" to:out_file
        local p = ogre_vert_pos[ov]
        format "                <position x=\"%\" y=\"%\" z=\"%\" />\n" p.x p.y p.z to:out_file
        local n = ogre_vert_normal[ov]
        format "                <normal x=\"%\" y=\"%\" z=\"%\" />\n" n.x n.y n.z to:out_file
        if options.exportAmbient or options.exportAlpha then (
            local c = ogre_vert_ambient[ov]
            local alpha = ogre_vert_alpha[ov]
            format "                <colour_diffuse value=\"% % % %\" />\n" c.x c.y c.z alpha to:out_file
        )
        if texture then (
            local uv = ogre_vert_texcoord[ov]
            format "                <texcoord u=\"%\" v=\"%\" />\n" uv.x (1-uv.y) to:out_file
        )
        if blend then (
            local sharpness = ogre_vert_sharpness[ov]
            format "                <texcoord u=\"%\" />\n" sharpness to:out_file
            for i = 0 to num_blend-1 do (
                local weight = ogre_vert_weights[i+1][ov]
                if (mod i 4) == 0 then (
                    if i > 0 then
                        format "\"/>\n" to:out_file
                    format "                <texcoord u=\"%\" " weight to:out_file
                )
                if (mod i 4) == 1 then 
                    format "v=\"%\" " weight to:out_file
                if (mod i 4) == 2 then 
                    format "w=\"%\" " weight to:out_file
                if (mod i 4) == 3 then 
                    format "x=\"%\" " weight to:out_file
            )
            format "/>\n" to:out_file
        )
        format "            </vertex>\n" to:out_file
    )

    format "        </vertexbuffer>\n" to:out_file
    format "    </sharedgeometry>\n" to:out_file

    format "    <submeshes>\n" to:out_file
    local counter = 0
    for submesh_mat_id in mat_ids do (
        local mat = selected.material
        if (classof mat) == MultiMaterial then (
            mat = mat.materialList[submesh_mat_id]
        )
        local matname = if mat == undefined then "BaseWhite" else mat.name
        format "        <submesh material=\"%\" use32bitindexes=\"%\">\n" matname (num_ogre_vertexes > 65535) to:out_file
        format "            <faces>\n" to:out_file
        for face = 1 to num_max_faces do (
            if (getFaceMatId m face) == submesh_mat_id then (
                GritRolloutOgreMain.exportProgress.value = 75.0 + (25.0*counter)/num_max_faces
                local fv = face_to_ogre_vert[face] - 1 -- ogre is zero-based
                format "                <face v1=\"%\" v2=\"%\" v3=\"%\" />\n" ((int)fv.x) ((int)fv.y) ((int)fv.z) to:out_file
                counter = counter + 1
            )
        )
        format "            </faces>\n" to:out_file
        format "        </submesh>\n" to:out_file
    )
    format "    </submeshes>\n" to:out_file

    format "    <vertexboneassignment>\n" to:out_file
    format "    </vertexboneassignment>\n" to:out_file

    --getDatas m options.exportUV options.UVchannels options.exportAmbient options.exportAlpha options.exportHelpers skin physique

    --WriteM options.exportUV options.UVchannels options.exportAmbient options.exportAlpha selected.material out_file

    local has_skeleton = false
    if (has_skeleton) then
        format "    <skeletonlink name=\"%\"/>\n" (out_name+".skeleton") to:out_file

    format "</mesh>\n" to:out_file

    format "Closing %.mesh.xml : \n" out_name
    close out_file;

    delete m ;        

    return true
) -- }}}

