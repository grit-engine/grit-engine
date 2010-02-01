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
		checkbox ccshadows "Cast shadows" across:2
		checkbox crshadows "Receive shadows"
		spinner sdrawd "Rendering distance" range:[0,5000,10] type:#integer fieldWidth:100
	)
	on ELCLASS pressed do
	(
	format "\nExporting class...\n" to:listener
	format "streamer:addClass(\"%%\",ColClass,{" mpath.text mnpath.text to:listener
	if ccshadows.tristate != 0 then
	(
	format "castShadows=true" to:listener
	)
	else ( format "castShadows=false" to:listener )
	if crshadows.tristate != 0 then
	(
	format ",receiveShadows=true" to:listener
	)
	else ( format "receiveShadows=false" to:listener )
	format ",renderingDistance=%" sdrawd.value to:listener
	format "})\n" to:listener
	format "\nFinished\n" to:listener
	)
	on SCLASS pressed do
	(
	if $ != undefined then
	for objs in $ do
		(
		setUserProp objs "obmpath" mpath.text
		setUserProp objs "obmnpath" mnpath.text
		setUserProp objs "obcshad" ccshadows.tristate
		setUserProp objs "obrshad" crshadows.tristate
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
		ccshadows.tristate = getUserProp objs "obcshad" 
		crshadows.tristate = getUserProp objs "obrshad"
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
	on expl pressed do
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
			
			format "streamer:addObject(\"%%\", %, %, %, {" tdir.text objs.name X Y Z to:listener
			if namexp.triState == 1 then
			(
				format "name=\"%\"" objs.name to:listener
			)
			if rotexp.triState == 1 then
			(
				if namexp.triState == 1 then
				(
					format ", rot=Vector3(%,%,%)" RX RY RZ to:listener
				)
				else
				format "rot=Vector3(%,%,%)" RX RY RZ to:listener
			)
			format "})\n" to:listener
		)
		format "Finished!\n" to:listener
	)
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
			format "streamer:addObject(\"%%\", %, %, %, {" mdir mname X Y Z to:listener
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
			format "})\n" to:listener
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