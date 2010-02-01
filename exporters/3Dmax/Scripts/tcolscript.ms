global nodeArray  -- used to store the nodes (meshes, primitives) created
global colStream

fn readTrimeshVerts &vArray= 
	(
	local inStr
		
	inStr = readLine colStream
	lineArgs = filterString (inStr as string) "; "
	while (lineArgs[1] != "}") do
		(
		append vArray [lineArgs[1] as float, lineArgs[2] as float, lineArgs[3] as float] 
		inStr = readLine colStream
		lineArgs = filterString (inStr as string) "; "
		) -- end while
	) -- end fn
	
fn readTrimeshfaces &fArray= 
	(
	local inStr
		
	inStr = readLine colStream
	lineArgs = filterString (inStr as string) "; "
	while (lineArgs[1] != "}") do
		(
		append fArray [(lineArgs[1] as integer)+1, (lineArgs[2] as integer)+1, (lineArgs[3] as integer)+1] 
		inStr = readLine colStream
		lineArgs = filterString (inStr as string) "; "
		) -- end while
	) -- end fn
	
fn readTrimesh =
	(
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
	) -- end fn

fn readHullVerts &vArray= 
	(
	local inStr
		
	inStr = readLine colStream
	lineArgs = filterString (inStr as string) "; "
	while (lineArgs[1] != "}") do
		(
		append vArray [lineArgs[1] as float, lineArgs[2] as float, lineArgs[3] as float] 
		inStr = readLine colStream
		lineArgs = filterString (inStr as string) "; "
		) -- end while
	) -- end fn
	
fn readHull =
	(
	local inStr, vertArray, faceArray

	inStr = readLine colStream -- "  vertexes {"
	
	vertArray = #(); 
	readHullVerts &VertArray
			

	inStr = readLine colStream -- "}"

	newMesh = mesh vertices:vertArray --faces:faceArray
	newMesh.Name = uniquename "Hull"
	newMesh -- return value
	) -- end fn
	
menu = newRolloutFloater "Grit .TCOL script" 320 363

Rollout maain "Main"
(
	group "Import/Export" (
		button imp "Import *.TCOL" width:146
		button exp "Export *.TCOL" width:146
		checkbox zexp "Move to 0.0 0.0 0.0 for export" align:#center
	)
	group "Attributes" (
		checkbox istat "Static" align:#center
		checkbox cymass "Export" across:2 
		spinner smass "Mass:" enabled:false
		checkbox cyinertia "Export" across:2 
		spinner sinertia "Inertia:" enabled:false
		checkbox cyldamp "Export" across:2 
		spinner sldamp "Linear Damping:" enabled:false
		checkbox cyadamp "Export" across:2 
		spinner sadamp "Angular Damping:" enabled:false
		checkbox cylsthrs "Export" across:2 
		spinner slsthrs "Linear sleep threshold:" enabled:false
		checkbox cyasthrs "Export" across:2 
		spinner sasthrs "Angular sleep threshold:" enabled:false
		checkbox cycmotthrs "Export" across:2
		spinner scmotthrs "CCD Motion Threshold:" enabled:false
		checkbox cycsphswrad "Export" across:2
		spinner scsphswrad "CCD Swept Sphere Radius:" enabled:false
		button resatb "Reset to default" width:116
	)
	on resatb pressed do
	(
	smass.value = 0.0 
	smass.enabled = false 
	sinertia.value = 0.0
	sinertia.enabled = false 
	sldamp.value = 0.5
	sldamp.enabled = false 
	sadamp.value = 0.0
	sadamp.enabled = false 
	slsthrs.value = 1.0
	slsthrs.enabled = false 
	sasthrs.value = 0.8
	sasthrs.enabled = false 
	scmotthrs.value = 0.0
	scmotthrs.enabled = false 
	scsphswrad.value = 0.0
	scsphswrad.enabled = false 
	
	istat.triState = 0
	
	cymass.triState = 0
	cyinertia.triState = 0
	cyldamp.triState = 0
	cyadamp.triState = 0
	cylsthrs.triState = 0
	cyasthrs.triState = 0
	cycmotthrs.triState = 0
	cycsphswrad.triState = 0
	)
	
	--This is about enabling or disabling the spinners if 'export' is selected
	on cymass changed theState do 
	(
		if cymass.triState == 0 then
		(		
		smass.enabled = false
		)
		else if cymass.triState == 1 then
		(		
		smass.enabled = true
		)
	)
	on cyinertia changed theState do 
	(
		if cyinertia.triState == 0 then
		(		
		sinertia.enabled = false
		)
		else if cyinertia.triState == 1 then
		(		
		sinertia.enabled = true
		)
	)
	on cyldamp changed theState do 
	(
		if cyldamp.triState == 0 then
		(		
		sldamp.enabled = false
		)
		else if cyldamp.triState == 1 then
		(		
		sldamp.enabled = true
		)
	)
	on cyadamp changed theState do 
	(
		if cyadamp.triState == 0 then
		(		
		sadamp.enabled = false
		)
		else if cyadamp.triState == 1 then
		(		
		sadamp.enabled = true
		)
	)
	on cylsthrs changed theState do 
	(
		if cylsthrs.triState == 0 then
		(		
		slsthrs.enabled = false
		)
		else if cylsthrs.triState == 1 then
		(		
		slsthrs.enabled = true
		)
	)
	on cyasthrs changed theState do 
	(
		if cyasthrs.triState == 0 then
		(		
		sasthrs.enabled = false
		)
		else if cyasthrs.triState == 1 then
		(		
		sasthrs.enabled = true
		)
	)
	on cycmotthrs changed theState do 
	(
		if cycmotthrs.triState == 0 then
		(		
		scmotthrs.enabled = false
		)
		else if cycmotthrs.triState == 1 then
		(		
		scmotthrs.enabled = true
		)
	)
	on cycsphswrad changed theState do 
	(
		if cycsphswrad.triState == 0 then
		(		
		scsphswrad.enabled = false
		)
		else if cycsphswrad.triState == 1 then
		(		
		scsphswrad.enabled = true
		)
	)
	--Finished the checkbox thing
	
	--Exporting
	on exp pressed do
	(
		savetcol = getSaveFileName types:"Grit collision file(*.tcol)|*.tcol|" 
		if savetcol != undefined then
		(		
			savetcol = createFile (savetcol)
			
		if $ != undefined then
		format "TCOL1.0\n\n" to:savetcol
		--attributes
		format "attributes {\n" to:savetcol
		format "\nStarted TCOL export\nTCOL 1.0\nName: %\nStarted exporting attributes...\n" savetcol to:listener
		if istat.triState == 1 then
		(
		format "static;\n" to:savetcol
		format "static\n" to:listener
		)
		if cymass.triState == 1 then
		(
		format "mass %;\n" smass.value to:savetcol
		format "mass: %\n" smass.value to:listener
		)
		if cyinertia.triState == 1 then
		(
		format "inertia %;\n" sinertia.value to:savetcol
		format "inertia: %\n" sinertia.value to:listener
		)
		if cyldamp.triState == 1 then
		(
		format "linear_damping %;\n" sldamp.value to:savetcol
		format "Linear Damping: %\n" sldamp.value to:listener
		)
		if cylsthrs.triState == 1 then
		(
		format "angular_damping %;\n" sadamp.value to:savetcol
		format "Angular Damping: %\n" sadamp.value to:listener
		)
		if cylsthrs.triState == 1 then
		(
		format "linear_sleep_threshold %;\n" slsthrs.value to:savetcol
		format "Linear sleep threshold: %\n" slsthrs.value to:listener
		)
		if cyasthrs.triState == 1 then
		(
		format "angular_sleep_threshold %;\n" sasthrs.value to:savetcol
		format "Angular sleep threshold: %\n" sasthrs.value to:listener
		)
		if cycmotthrs.triState == 1 then
		(
		format "ccd_motion_threshold %;\n" scmotthrs.value to:savetcol
		format "CCD Motion Threshold: %\n" scmotthrs.value to:listener
		)
		if cycsphswrad.triState == 1 then
		(
		format "ccd_swept_sphere_radius %;\n" scsphswrad.value to:savetcol
		format "CCD Swept Sphere Radius: %\n" scsphswrad.value to:listener
		)
		format "}\n\n" to:savetcol
		format "Finished exporting attributes\n" to:listener
		
		--compounds
		format "compound {\n" to:savetcol
		format "Started exporting compounds...\n" to:listener
		for objs in $ do
		(
			OX = objs.pos.x
			OY = objs.pos.Y
			OZ = objs.pos.Z
			if zexp.triState == 1 then
			(
			objs.pos.x = 0.0
			objs.pos.y = 0.0
			objs.pos.z = 0.0
			format "Moved % from % % % to %\n" objs.name OX OY OZ objs.pos to:listener
			)
			if classof(objs) == Box then
			(
				if objs.material != undefined then
				(
				if objs.material.physmat != undefined then
				(
				matid = objs.material.physmat))
				else (
				matid = 0
				format "% doesn't have a phys mat assigned!!!\n assigning FrictionLess...\n" objs.name to:listener
				)
				L = objs.length
				W = objs.width
				H = objs.height
				X = objs.pos.X
				Y = objs.pos.Y
				Z = objs.pos.Z + (objs.height/2)
				format "box {\nmaterial 0x%;\ncentre % % %;\ndimensions % % %;\n" matid X Y Z L W H to:savetcol
				format "}\n" to:savetcol
				format "Exported box at % % % length:% width:% height:%\n" X Y Z L W H to:listener
			)
			if classof(objs) == Cylinder then
			(
				if objs.material != undefined then
				(
				if objs.material.physmat != undefined then 
				(
				matid = objs.material.physmat))
				else (
				matid = 0
				format "% doesn't have a phys mat assigned!!!\n assigning FrictionLess...\n" objs.name to:listener
				)
				RX = objs.radius*2
				RY = objs.scale.Y*objs.radius*2
				H = objs.height
				X = objs.pos.X
				Y = objs.pos.Y
				Z = objs.pos.Z + (objs.height/2)
				format "cylinder {\nmaterial 0x%;\ncentre % % %;\ndimensions % % %;\n" matid X Y Z RX RY H to:savetcol
				format "}\n" to:savetcol
				format "Exported cylinder at % % % radius X:% radius Y:% Height:%\n" X Y Z RX RY H to:listener
			)
			if classof(objs) == Cone then
			(
				if objs.material != undefined then
				(
				if objs.material.physmat != undefined then 
				(
				matid = objs.material.physmat))
				else (
				matid = 0
				format "% doesn't have a phys mat assigned!!!\n assigning FrictionLess...\n" objs.name to:listener
				)
				R = objs.radius1
				H = objs.height
				X = objs.pos.X
				Y = objs.pos.Y
				Z = objs.pos.Z + (objs.height/2)
				format "cone {\nmaterial 0x%;\ncentre % % %;\nradius %;\nheight %;\n" matid X Y Z R H to:savetcol
				format "}\n" to:savetcol
				format "Exported cone at % % % radius %\n height %;\n" X Y Z R H  to:listener
			)
			if classof(objs) == Sphere then
			(
				if objs.material != undefined then
				(
				if objs.material.physmat != undefined then
				(
				matid = objs.material.physmat))
				else (
				matid = 0
				format "% doesn't have a phys mat assigned!!!\n assigning FrictionLess...\n" objs.name to:listener
				)
				R = objs.radius
				X = objs.pos.X
				Y = objs.pos.Y
				Z = objs.pos.Z
				format "sphere {\nmaterial 0x%;\ncentre % % %;\nradius %;\n" matid X Y Z R to:savetcol
				format "}\n" to:savetcol
				format "Exported sphere at % % % radius:%\n" X Y Z R to:listener
			)
			format "Finished exporting compounds\n" to:listener
			format "}\n" to:savetcol
			
			--trimesh
			if classof objs.baseobject == Editable_Poly then
			(
			theEP = selection[1].baseobject
			format "trimesh {\n" to:savetcol
			format "Started exporting trimesh...\n" to:listener
			tmesh = snapshotAsMesh objs
			num_verts = tmesh.numverts 
			num_faces = tmesh.numfaces
			format "Verts: %\nTris: %\n" num_verts num_faces to:listener
			format "vertexes {\n" to:savetcol
				for v = 1 to num_verts do
				(
				 vert = getVert tmesh v
				 format "% % %;\n" vert.x vert.y vert.z to:savetcol
				 continue
				)
				format "verts exporting finished!\n" to:listener
				format "}\nfaces {\n" to:savetcol
				for f = 1 to num_faces do
				(
				face = getFace tmesh f
				faceord = face-1
				--format "face: %\n" faceord to:listener
				matnmb = getFaceMatId tmesh f
				matname = objs.material
				mats = matname.materialIDlist[matnmb]
				mat = matname.materialList[mats]
				--format "material: %\n" mat.name to:listener
				materialClass = classof mat
				if materialClass != GRIT_PHYSMAT then
				(
				matid = 0
				format "Face '%' doesn't have a phys mat assigned!!!\nAssigning FrictionLess...\n" f to:listener
				)
				else (
				matid = mat.physmat
				)
				format "% % % 0x%;\n" (faceord[1] as integer) (faceord[2] as integer) (faceord[3] as integer) matid to:savetcol
				)
				format "faces exporting finished!\n" to:listener
				delete tmesh
				format "}\n" to:savetcol
				format "Finished exporting trimesh\n" to:listener
			)
			format "}\n" to:savetcol
			objs.pos.X = OX
			objs.pos.Y = OY
			objs.pos.Z = OZ
			format "Moved % back to %\n" objs.name objs.pos to:listener
		)
		format "finished!\n" to:listener
		close savetcol
		)
	)
	
	--Importing
	on imp pressed do
	(
	colopenname = getOpenFileName caption:"Open TCOL" types:"Open Grit collision file (*.tcol)|*.tcol|"
	colStream = undefined
	if colopenname != undefined then
		colStream = openfile colopenname
	if colStream != undefined do
		(
		lc = 1
		while not eof colStream do
			(
			--format "LC = %\n" lc; lc += 1
			inputline = readLine colStream --We start reading the file
			lineArgs = filterString inputline "; "
			
			if lineArgs.count == 0 then
				continue
				
			if lineArgs[1] == "/*" then
				(
					while (lineArgs[1] != "*/") do
					(
					)
				continue
				)
			
			if lineArgs[1] == "TCOL1.0" then
				(
				format "\nStarted TCOL import\nTCOL 1.0\nName: %\n" colopenname to:listener
				continue
				)
				
			if lineArgs[1] == "//" then
				(
				commnt = lineArgs[2] 
				format "Comment: %\n" commnt to:listener
				continue
				) 
				
			if lineArgs[1] == "attributes" then
				(
				format "Importing atributes\n" to:listener
				continue
				) 
				
			if lineargs[1] == "mass" then 
				(
				cymass.triState = 1
				massv = (lineArgs[2] as float) 
				smass.value = massv
				format "Mass: %\n" (lineArgs[2] as float)  to:listener
				continue
				)
				
			if lineargs[1] == "static" then 
				(
				istat.triState = 1
				format "Static\n" to:listener
				continue
				)
				
			if lineargs[1] == "inertia" then
				(
				cyinertia.triState = 1
				inertiav = (lineArgs[2] as float) 
				sinertia.value = inertiav
				format "Inertia: %\n" (lineArgs[2] as float)  to:listener
				continue
				)

			if lineargs[1] == "linear_damping" then
				(
				cyldamp.triState = 1
				ldampv = (lineArgs[2] as float) 
				sldamp.value = ldampv
				format "Linear Damping: %\n" (lineArgs[2] as integer) to:listener
				continue
				)
				
			if lineargs[1] == "angular_damping" then
				(
				cyadamp.triState = 1
				adampv = (lineArgs[2] as float) 
				sadamp.value = adampv
				format "Angular Damping: %\n" (lineArgs[2] as integer) to:listener
				continue
				)
				
			if lineargs[1] == "linear_sleep_threshold" then
				(
				cylsthrs.triState = 1
				lsthrsv = (lineArgs[2] as float) 
				slsthrs.value = lsthrsv
				format "Linear sleep threshold: %\n" (lineArgs[2] as integer) to:listener
				continue
				)
				
			if lineargs[1] == "angular_sleep_threshold" then
				(
				cyasthrs.triState = 1
				asthrsv = (lineArgs[2] as float) 
				sasthrs.value = asthrsv
				format "Angular sleep threshold: %\n" (lineArgs[2] as integer) to:listener
				continue
				)
				
			if lineargs[1] == "ccd_motion_threshold" then
				(
				cycmotthrs.triState = 1
				cmotthrsv = (lineArgs[2] as float) 
				scmotthrs.value = cmotthrsv
				format "CCD Motion Threshold: %\n" (lineArgs[2] as integer) to:listener
				continue
				)
				
			if lineargs[1] == "ccd_swept_sphere_radius" then
				(
				cycsphswrad.triState = 1
				csphswradv = (lineArgs[2] as float) 
				scsphswrad.value = csphswradv
				format "CCD Swept Sphere Radius: %\n" (lineArgs[2] as integer) to:listener
				continue
				)
 
			if lineArgs[1] == "}" do 
				(
				format "Finished \n" to:listener 
				continue
				)
				
			-- Trimesh
				
			if lineArgs[1] == "trimesh" then
				(
				format "Importing trimesh\n" to:listener
				readTrimesh()
				continue
				) 
				
			if lineArgs[1] == "hull" then
				(
				format "Ignoring hull\n" to:listener
				--readHull()
				continue
				) 
				
			-- Compounds
				
			if lineArgs[1] == "compound" then
				(
				format "Importing compounds\n" to:listener
				continue
				) 
				
			if lineArgs[1] == "box" then
				(
					while (lineArgs[1] != "}") do
					(
						inStr = readLine colStream
						lineArgs = filterString (inStr as string) "; "
						if lineArgs[1] == "material" then
						(
							M = lineArgs[2]
							continue
						)
						if lineArgs[1] == "centre" then
						(
							X = lineArgs[2] as float
							Y = lineArgs[3] as float
							ZG = lineArgs[4] as float
							continue
						)
						if lineArgs[1] == "dimensions" then
						(
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
				
			if lineArgs[1] == "cylinder" then
				(
					while (lineArgs[1] != "}") do
					(
						inStr = readLine colStream
						lineArgs = filterString (inStr as string) "; "
						if lineArgs[1] == "material" then
						(
							M = lineArgs[2]
							continue
						)
						if lineArgs[1] == "centre" then
						(
							X = lineArgs[2] as float
							Y = lineArgs[3] as float
							ZG = lineArgs[4] as float
							continue
						)
						if lineArgs[1] == "dimensions" then
						(
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
				
			if lineArgs[1] == "cone" then
				(
					while (lineArgs[1] != "}") do
					(
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
							ZG = lineArgs[4] as float
							continue
						)
						if lineArgs[1] == "radius" then
						(
							R = lineArgs[2] as float
							continue
						)
						if lineArgs[1] == "height" then
						(
							H = lineArgs[2] as float
							continue
						)
						Z=ZG-H/2
						c = cone radius1:R radius2:0 height:H pos:[X,Y,Z]
						format "Imported % at % % % radius:% height:% material:%\n" c.name X Y Z c.radius1 c.height M to:listener
					) -- end while
				continue
				)

			if lineArgs[1] == "sphere" then
				(
					while (lineArgs[1] != "}") do
					(
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
		
	) -- this ends the function
	
)

Rollout help "Help"
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
)
addRollout maain menu
addRollout help menu