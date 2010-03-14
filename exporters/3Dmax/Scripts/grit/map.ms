cool = newRolloutFloater "Grit Helper" 250 500

Rollout clasz "Classes"
(
	group "Get/Set/Export" (
		button SCLASS "Set Class" width:146
		button RCLASS "Retrieve class" width:146
		button ECLASS "Export selected classes" width:130
		button ELCLASS "Export this class to listener" width:130
	)
	group "Path/Name" (
		edittext mpath "Path" fieldWidth:160
		edittext mnpath "Mesh name" fieldWidth:140
	)
	group "Attributes" (
		spinner sdrawd "Rendering distance" range:[0,5000,10] type:#integer fieldWidth:100
	)
	-- Exports to listener!!
	on ELCLASS pressed do
	(
	format "\nExporting class...\n" to:listener
	format "Class \"%%\" (ColClass) {" mpath.text mnpath.text to:listener
	format "renderingDistance=%" sdrawd.value to:listener
	format "}\n" to:listener
	format "\nFinished\n" to:listener
	)
	-- User properties, sets and retrieve
	on SCLASS pressed do
	(
	if $ != undefined then
	for objs in $ do
		(
		setUserProp objs "obmpath" mpath.text
		setUserProp objs "obmnpath" mnpath.text
		setUserProp objs "obdrawd" sdrawd.value
		)
	)
	on RCLASS pressed do
	(
	if $ != undefined then
	for objs in $ do
		(
		if getUserProp objs "obmpath" == undefined then (
		mpath.text = ""
		)
		mpath.text = getUserProp objs "obmpath"
		mnpath.text = getUserProp objs "obmnpath" 
		sdrawd.value = getUserProp objs "obdrawd"
		)
	)
)

Rollout placz "Placement"
(
	group "Fast Export" (
		button expl "Export to listener" width:146
		checkbox namexp "Export name" across:2
		checkbox rotexp "Export rot" align:#right   
		edittext tdir "Directory with / " fieldWidth:150 labelOnTop:true
	)
	group "Export Object placement" (
		button oexpl "Export to listener" width:146
		button oexp "Export to file" width:146
		checkbox onamexp "Export name" across:2
		checkbox orotexp "Export rot" align:#right   
	)
	--Exports to file
	on oexp pressed do
	(
	format "\nStarting exporting coords\n" to:listener
	objfl = getSaveFileName types:"LUA File (*.lua)|*.lua|" 
		if objfl != undefined then
		(		
			objfl = createFile (objfl)
			format "--lua file generated with JostVice's GritMap script\n--Placement\n" to:objfl
		if $ != undefined then
		for objs in $ do
			(
				X = objs.pos.X 
				Y = objs.pos.Y 
				Z = objs.pos.Z
				RX = (in coordsys world objs.rotation.x_rotation )
				RY = (in coordsys world objs.rotation.y_rotation )
				RZ = (in coordsys world objs.rotation.z_rotation )
				
				format "object \"%%\" (%, %, %) {" tdir.text objs.name X Y Z to:objfl
				if namexp.triState == 1 then
				(
					format "name=\"%\"" objs.name to:listener
				)
				if rotexp.triState == 1 then
				(
					if namexp.triState == 1 then
					(
						format ", rot=Vector3(%,%,%)" RX RY RZ to:objfl
					)
					else
					format "rot=Vector3(%,%,%)" RX RY RZ to:objfl
				)
				format "}\n" to:objfl
			)
			format "Finished!\n" to:listener
		close objfl
		)
	)
	--Exports to listener
	on oexpl pressed do
	(
	format "\nStarting exporting coords\n" to:listener
	if $ != undefined then
	for objs in $ do
		(
			X = objs.pos.X 
			Y = objs.pos.Y 
			Z = objs.pos.Z
			RX = (in coordsys world objs.rotation.x_rotation )
			RY = (in coordsys world objs.rotation.y_rotation )
			RZ = (in coordsys world objs.rotation.z_rotation )
			mdir = getUserProp objs "obmpath"
			mname = getUserProp objs "obmnpath" 
			
			format "Object \"%%\" (%, %, %) {" mdir mname X Y Z to:listener
			if onamexp.triState == 1 then
			(
				format "name=\"%\"" objs.name to:listener
			)
			if orotexp.triState == 1 then
			(
				if onamexp.triState == 1 then
				(
					format ", rot=Vector3(%,%,%)" RX RY RZ to:listener
				)
				else
				format "rot=Vector3(%,%,%)" RX RY RZ to:listener
			)
			format "}\n" to:listener
		)
		format "Finished!\n" to:listener
	)
)

Rollout help "Help"
(
	group "Help" (
		label lab1 "*Grit utility tool"
		label bspc " "
	)
)
	
addRollout clasz cool
addRollout placz cool
addRollout help cool