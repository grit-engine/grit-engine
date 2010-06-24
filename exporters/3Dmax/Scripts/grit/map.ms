cool = newRolloutFloater "Grit Mapping Helper" 250 600

Rollout clasz "Classes"
(
	group "" (
		bitmap IMGCLASS fileName:"ui/Classes.bmp"
		button SCLASS "Set Class" width:146
		button RCLASS "Retrieve class" width:146
		button ECLASS "Export selected classes" width:146
		button ELCLASS "Export this class to listener" width:146
	)
	group "" (
		bitmap IMGPROP fileName:"ui/Properties.bmp"
		edittext mpath "Path" fieldWidth:160
		edittext mnpath "Mesh name" fieldWidth:140
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
	group "" (
		bitmap IMGFEXP fileName:"ui/FastExp.bmp"
		button expl "Export to listener" width:146
		checkbox namexp "Export name" across:2
		checkbox rotexp "Export rot" align:#right   
		edittext tdir "Directory with / " fieldWidth:150 labelOnTop:true
	)
	group "" (
		bitmap IMGOEXP fileName:"ui/ObjExp.bmp"
		button oexpl "Export to listener" width:146
		button oexp "Export to file" width:146
		checkbox onamexp "Export name" across:2
		checkbox orotexp "Export rot" align:#right   
	)
	--Exports to file
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
			mdir = tdir.text
			mname = objs.name 
			
			format "object \"%%\" (%, %, %) {" mdir mname X Y Z to:listener
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
			
			format "object \"%%\" (%, %, %) {" mdir mname X Y Z to:listener
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
	group "" (
		bitmap IMGABO fileName:"ui/about.bmp"
		label lab1 "Grit Map Helper"
		label bspc "Coded by JostVice and help from many others."
		label bspc2 "Use this to speed up your workflow in "
		label bspc3 "large scenes with lots of instances. "
		label bspc4 "Set classes and export pos and rot fast."
		label VERSION "1.0.0"
		hyperlink LBLOG "Grit Blog" url:"http://gritengine.com/" color:orange across:3
		hyperlink LFORM "Grit Forum" url:"http://gritengine.com/game-engine-forum/" color:orange
		hyperlink LWIKI "Grit Wiki" url:"http://gritengine.com/game-engine-wiki/" color:orange
	)
)
	
addRollout clasz cool
addRollout placz cool
addRollout help cool
