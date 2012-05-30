global grit_current_map_floater = undefined

global GritRolloutClass
global GritRolloutPlace
global GritRolloutAbout

--cool = newRolloutFloater "Grit Mapping Helper" 250 600

Rollout GritRolloutClass "Classes" -- {{{
(
	group "" (
		bitmap IMGCLASS fileName:"ui/Classes.bmp"
		button SCLASS "Set" width:60 across:2 align:#center
		button RCLASS "Retrieve" width:60 align:#center
		progressbar PBCEXP "Progressbar:" value:0 color:orange
		button ECLASS "Export selected classes" width:146 align:#center
		label CE1 "Exports selected classes to listener"
		button ELCLASS "Export this class to listener" width:146 align:#center
		label CE2 "Exports the class in the properties dialog"
		label CE22 "to listener"
		button EOCLASS "Export selected classes to a file" width:156 align:#center
		label CE3 "Exports selected classes to a *.lua file"
		label BLN1 " "
		button ELLIGHT "Export selected lights to listener" width:156 align:#center
		label CE4 "Exports selected lights (only omni and "
		label CE5 "free spot) to listener."
		label CE6 "Remember to have lights linked to a parent!"
		label BLN2 " "
		bitmap IMGPROP fileName:"ui/Properties.bmp"
		edittext mpath "Folder Path" fieldWidth:139 align:#center
		edittext mnpath "Mesh name" fieldWidth:140 align:#center
		spinner sdrawd "Rendering distance" range:[0,5000,10] type:#integer fieldWidth:100 align:#center
		dropdownlist ctype "Class type" items:#("ColClass", "BaseClass") width:80 align:#center
	)
	-- Exports to listener!!
	on ELCLASS pressed do
	(
	PBCEXP.value = 0
	format "\nExporting class...\n" to:listener
	PBCEXP.value = 25
	format "class \"%%\" (%) {" mpath.text mnpath.text ctype.selected to:listener
	format "renderingDistance=%" sdrawd.value to:listener
	PBCEXP.value = 75
	format "}\n" to:listener
	PBCEXP.value = 100
	format "\nFinished\n" to:listener
	PBCEXP.value = 0
	)
	on ECLASS pressed do
		(
		format "\nExporting class...\n" to:listener
		if $ != undefined then
		--PBCNMB = $.count / 100
		for objs in $ do
		(
			--PBCEXP.value = (PBCEXP.value + PBCNMB)
			mdir = getUserProp objs "obmpath"
			mname = getUserProp objs "obmnpath" 
			mdrawd = getUserProp objs "obdrawd"
			mctype = getUserProp objs "obctype"
		
			format "class \"%%\" (%) {" mdir mname mctype to:listener
			format "renderingDistance=%" mdrawd to:listener
			format "}\n" to:listener
		)
		PBCEXP.value = 0
		format "\nFinished\n" to:listener
		)
		on EOCLASS pressed do
		(
		format "\nStarting exporting classes\n" to:listener
		objfl = getSaveFileName types:"LUA File (*.lua)|*.lua|" 
		if objfl != undefined then
			(		
				objfl = createFile (objfl)
				format "--lua file generated with JostVice's GritMap script\n--Classes\n" to:objfl
				if $ != undefined then
				--PBCNMB = $.count / 100
				for objs in $ do
				(
					--PBCEXP.value = (PBCEXP.value + PBCNMB)
					mdir = getUserProp objs "obmpath"
					mname = getUserProp objs "obmnpath" 
					mdrawd = getUserProp objs "obdrawd"
					mctype = getUserProp objs "obctype"
						
					format "class \"%%\" (%) {" mdir mname mctype to:objfl
					format "renderingDistance=%" mdrawd to:objfl
					format "}\n" to:objfl
				)
				PBCEXP.value = 0
				format "Finished!\n" to:listener
				close objfl
			)
		)
		on ELLIGHT pressed do
		(
			PBCEXP.value = 0
			format "\nExporting lights...\n" to:listener
			if $ != undefined then
			format "\nlights = {" to:listener
			for lightsz in $ do
			--(lightsz as array).count = totallights
			(
			if lightsz.type == #omni do (
				--format "\nexporting omni lights\n" to:listener	
				X = (in coordsys parent lightsz.pos.x )
				Y = (in coordsys parent lightsz.pos.y )
				Z = (in coordsys parent lightsz.pos.z )

				lname = lightsz.name 
				lcolor = lightsz.rgb
				
				lcolorred= (lcolor.red / 255)
				lcolorgreen= (lcolor.green / 255)
				lcolorblue= (lcolor.blue / 255)
				
				lspec = lightsz.rgb
				
				lspecred= (lspec.red / 255)
				lspecgreen= (lspec.green / 255)
				lspecblue= (lspec.blue / 255)
				
				lrange = lightsz.DecayRadius
				
				format "\n{ pos=vector3(%, %, %), diff=vector3(%,%,%), spec=vector3(%,%,%), range=% }," X Y Z lcolorred lcolorgreen lcolorblue lspecred lspecgreen lspecblue lrange to:listener
			)
			PBCEXP.value = 50
			if lightsz.type == #freeSpot do (
				--format "\nexporting spot lights\n" to:listener	
				X = (in coordsys parent lightsz.pos.x )
				Y = (in coordsys parent lightsz.pos.y )
				Z = (in coordsys parent lightsz.pos.z )
				--As grit's spotlight at rotation 0,0,0 points at +y and max at -z,
				-- we multiply max orientation by quat(90,vector3(-1,0,0)) (90 degrees around x axis negative)
				--i dont know it just works :D
				MAX_ORIENTATION = (in coordsys parent lightsz.rotation)
				R = MAX_ORIENTATION * (quat 0.7071068 -0.7071068 0 0)
				--RX = (R.x)
				--RY = (in coordsys parent lightsz.rotation.y )
				--RZ = (in coordsys parent lightsz.rotation.z )
				--RW = (in coordsys parent lightsz.rotation.w )
				
				lname = lightsz.name 
				lcolor = lightsz.rgb
				
				lcolorred= (lcolor.red / 255)
				lcolorgreen= (lcolor.green / 255)
				lcolorblue= (lcolor.blue / 255)
				
				lspec = lightsz.rgb
				
				lspecred= (lspec.red / 255)
				lspecgreen= (lspec.green / 255)
				lspecblue= (lspec.blue / 255)
				
				lrange = lightsz.DecayRadius
				loangle = lightsz.falloff
				liangle = lightsz.hotspot
				
				format "\n{ pos=vector3(%, %, %), diff=vector3(%,%,%), spec=vector3(%,%,%), range=%, iangle=%, oangle=%, aim=quat(%,%,%,%) }," X Y Z lcolorred lcolorgreen lcolorblue lspecred lspecgreen lspecblue lrange liangle loangle R.x R.y R.z R.w to:listener
				)
			)
			PBCEXP.value = 100
			format "\n}\nfinished lights...\n" to:listener	 
			PBCEXP.value = 0
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
		setUserProp objs "obctype" ctype.selected
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
		
		if getUserProp objs "obctype" == undefined then (
		mpath.text = "ColClass"
		)
		else
		ctype.selected = getUserProp objs "obctype"
		)
	)
) --}}}

Rollout GritRolloutPlace "Placement" -- {{{
(
	group "" (
		bitmap IMGFEXP fileName:"ui/FastExp.bmp"
		label FE1 "Export only the selected object"
		label FE2 "using the properties below, ignoring"
		label FE3 "any class properties you may have Set"
		button expl "Export to listener" width:146  align:#center
		checkbox namexp "Export name" across:2 align:#center
		checkbox rotexp "Export rotation" align:#center   
		edittext tdir "Folder path" fieldWidth:165 labelOnTop:true align:#center
		label OE1 "Directory where the object is."
		label OE2 "Remember to put / at the end."
		label BLN2 " "
		bitmap IMGOEXP fileName:"ui/ObjExp.bmp"
		label EE1 "Export the selected object with the"
		label EE2 "Class properties that you've Set"
		progressbar PBOEXP "Progressbar:" value:0 color:orange
		button oexpl "Export to listener" width:146 align:#center
		button oexp "Export to file" width:146 align:#center
		checkbox onamexp "Export name" across:2 align:#center
		checkbox orotexp "Export rotation" align:#center  
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
					format ", rot=euler(%,%,%)" RX RY RZ to:listener
				)
				else
				format "rot=euler(%,%,%)" RX RY RZ to:listener
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
		--PBONMB = $.count / 100
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
				
				format "object \"%%\" (%, %, %) {" mdir mname X Y Z to:objfl
				if onamexp.triState == 1 then
				(
					format "name=\"%\"" objs.name to:listener
				)
				if orotexp.triState == 1 then
				(
					if onamexp.triState == 1 then
					(
						format ", rot=euler(%,%,%)" RX RY RZ to:objfl
					)
					else
					format "rot=euler(%,%,%)" RX RY RZ to:objfl
				)
				--PBOEXP.value = (PBOEXP.value + PBONMB)
				format "}\n" to:objfl
			)
			--PBOEXP.value = 0
			format "Finished!\n" to:listener
		close objfl
		)
	)
	--Exports to listener
	on oexpl pressed do
	(
	format "\nStarting exporting coords\n" to:listener
	if $ != undefined then
	--PBONMB = $.count / 100
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
					format ", rot=euler(%,%,%)" RX RY RZ to:listener
				)
				else
				format "rot=euler(%,%,%)" RX RY RZ to:listener
			)
			format "}\n" to:listener
			--PBOEXP.value = (PBOEXP.value + PBONMB)
		)
		--PBOEXP.value = 0
		format "Finished!\n" to:listener
	)
) -- }}}

Rollout GritRolloutAbout "Help" -- {{{
(
	group "" (
		bitmap IMGABO fileName:"ui/about.bmp"
		label lab1 "Grit Map Helper"
		label bspc "Coded by JostVice and help from many others."
		label bspc2 "Use this to speed up your workflow in "
		label bspc3 "large scenes with lots of instances. "
		label bspc4 "Set classes and export pos and rot fast."
		label VERSION "1.0.0"
		hyperlink LBLOG "Grit Blog" address:"http://gritengine.com" color:orange across:3 align:#center
		hyperlink LFORM "Grit Forum" address:"http://gritengine.com/game-engine-forum" color:orange align:#center
		hyperlink LWIKI "Grit Wiki" address:"http://gritengine.com/game-engine-wiki" color:orange align:#center
	)
) -- }}}
	
--addRollout clasz cool
--addRollout placz cool
--addRollout help cool

--callbacks.removescripts id:#GritTcolSelection
--callbacks.addScript #selectionSetChanged "GritRolloutTcolMain.update()" id:#GritTcolSelection

macroScript showGritMapHelper
    category:"Grit Tools"
    internalCategory:"Grit Tools"
    buttonText:"Grit Mapping Helper"
    tooltip:"Grit Mapping helper (Class & Placement assist)"
    Icon:#("Grit",3)
(
    if grit_current_map_floater != undefined then (
        closeRolloutFloater grit_current_map_floater
    )
    grit_current_map_floater = newRolloutFloater "Grit Mapping Helper" 250 600

    addRollout GritRolloutClass grit_current_map_floater
	addRollout GritRolloutPlace grit_current_map_floater
	addRollout GritRolloutAbout grit_current_map_floater

    --GritRolloutTcolMain.update()
)
