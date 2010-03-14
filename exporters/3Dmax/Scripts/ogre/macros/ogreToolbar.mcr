fileIn (ogre_exporter_dir+"ogre/lib/ogreSkeletonLib_usefulfns.ms")
fileIn (ogre_exporter_dir+"ogre/lib/ogreSkeletonLib_meshfns.ms")
fileIn (ogre_exporter_dir+"ogre/lib/ogreBipedLib.ms")

macroScript showOgreExportTools
	category:"Grit Tools"
	internalCategory:"Grit Tools"
	buttonText:"Grit Mesh Exporter"
	tooltip:"Grit Mesh Exporter"
	Icon:#("Maintoolbar",49)
(

	rollout OgreExportObject "MAIN" width:272 height:235 -- {{{
	(
		button selectMesh "Select Model..." pos:[8,8] width:248 height:32
		GroupBox grp1 "Output File Name (without extension)" pos:[8,80] width:254 height:75
		edittext editFilename "" pos:[16,96] width:232 height:22
		button chooseFilename "Browse" pos:[96,128] width:82 height:20 toolTip:"chooose the name of your output files"
		button Export "Export !" pos:[29,186] width:214 height:40 toolTip:"export your mesh, skeleton ......"
		progressBar exportProgress "" pos:[8,168] width:250 height:10
		
		
		-------------------------------------
		-- EXPORT ---------------------------
		-------------------------------------

		on OgreExportObject open do
		(
			lastFile = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastFile"
			lastAnimName = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastAnimName"
			lastfirstFrame = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastFrameStart"
			lastlastFrame = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastFrameEnd"
			lastAnimLength = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastAnimLength"
			
			editFilename.text = lastFile ;
			OgreExportMesh.CBexportUV.checked = true ;

			select = getCurrentSelection() ;
			if ((select[1]!=undefined) and (iskindof select[1] GeometryClass)) then
			(
				selectMesh.text = select[1].name ;
				OgreExportMesh.CBexportMesh.enabled = true;
				OgreExportMesh.update_enabled()

				OgreExportMesh.CBexportMesh.checked = true;
				OgreExportAnimation.CBexportSkeleton.enabled = (getSkin(select[1]) != undefined) or (getPhysique(select[1]) != undefined) ;
			)
		)
		on selectMesh pressed do
		(
			max tool hlist ;
			select = getCurrentSelection() ;
			if ((select[1]!=undefined) and (iskindof select[1] GeometryClass)) then
			(
				selectMesh.text = select[1].name ;
			
				-- options accessible ou non
				OgreExportMesh.CBexportMesh.enabled = true;
				OgreExportMesh.update_enabled()
				OgreExportMesh.CBexportMesh.checked = true;
				OgreExportAnimation.CBexportSkeleton.enabled = (getSkin(select[1]) != undefined) or (getPhysique(select[1]) != undefined) ;
			)
		)
		on chooseFilename pressed do
		(
			filename = getSaveFileName types:"All Files(*.*)|*.*|" ;			
			if (filename != undefined) then
			(
				editFilename.text = filename ;
			)
		)
		on Export pressed do
		(
			sliderTime = 0 ;
			m = getNodeByName selectMesh.text ;
			if (editFilename.text == "" or m == undefined ) then
			(
				messageBox "You have to choose a filename and a valid object." ;
			)
			else
			(
				clearlistener() ;
				
				
				Options = exportOptions flipNormal:false exportColours:false exportAlpha:false exportUV:false ;
				exportingMeshDone = false ;
				exportingSkelDone = false ;
				-- sets options
				---------------
				if (OgreExportMesh.CBflipnormals.checked and OgreExportMesh.CBflipnormals.enabled) then
					Options.flipNormal = true ;
				if (OgreExportMesh.CBexportColor.checked and OgreExportMesh.CBexportColor.enabled) then
					Options.exportColours = true ;
				if (OgreExportMesh.CBexportAlpha.checked and OgreExportMesh.CBexportAlpha.enabled) then
					Options.exportAlpha = true ;
				if (OgreExportMesh.CBexportUV.checked and OgreExportMesh.CBexportUV.enabled) then
				(
					Options.exportUV = true ;	
					Options.UVchannels = OgreExportMesh.SPchannels.value;	
				)
				Options.exportHelpers = (OgreExportAnimation.CBexporthelpers.enabled and OgreExportAnimation.CBexporthelpers.checked);
				

				-- exports mesh
				---------------
				if (OgreExportMesh.CBexportMesh.enabled and OgreExportMesh.CBexportMesh.checked) then
				(
                    setINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastFile" editFilename.text	
					--progressStart "Mesh Export" ;
					exportingMeshDone = writeMesh m Options (editFilename.text) ;
					--progressStop ;
				)
			
				-- exports skeleton
				-------------------
				if (OgreExportAnimation.CBexportSkeleton.enabled and OgreExportAnimation.CBexportSkeleton.checked) then
				(
					Options.sampleRate = OgreExportAnimation.SPsamplerate.value ;
					Options.ikSampleRate = OgreExportAnimation.SPiksamplerate.value ;
				
					exportingSkelDone =	writeSkeleton m Options Anims editFilename.text ;
				)
				
				-- post traitement
				------------------
				if (exportingMeshDone or exportingSkelDone) then 
				(					
					if (OgreExportOptions.CBconvertXML.checked) then
					(
                        OptionConv = "";
                        mediaPath = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Directories" "MediaPath"
                
                        if (exportingMeshDone) then 
                        (
                            if (OgreExportOptions.CBgenerateedges.enabled==false or OgreExportOptions.CBgenerateedges.checked==false) then
                                OptionConv = OptionConv + " -e";
                            if (OgreExportOptions.CBgeneratetangents.enabled and OgreExportOptions.CBgeneratetangents.checked) then
                            (
                                if (OgreExportMesh.CBexportUV.enabled and OgreExportMesh.CBexportUV.checked and (OgreExportMesh.SPchannels.value>0)) then
                                    OptionConv = OptionConv + " -t";
                                else
                                    messageBox "Tangent vectors cannot be generated without UV sets. Generation will be deactivated." ;
                            )
                            if (OgreExportOptions.CBgeneratelod.enabled and OgreExportOptions.CBgeneratelod.checked) then
                                OptionConv = OptionConv + " -l " + (OgreExportOptions.SPlodlevels.value as string) + " -d " + (OgreExportOptions.SPloddistance.value as string) + " -p " + (OgreExportOptions.SPlodreduction.value as string);
                            
                            DOSCommand (OgreExportOptions.editXMLconverter.text + OptionConv + " \"" + editFilename.text + ".mesh.xml\" \"" + editFilename.text + ".mesh\"") ;
                            DOSCommand ("copy \"" + editFilename.text + ".mesh\" \"" + mediaPath + "\"") ;
                        )
                        if (exportingSkelDone) then 
                        (
                            DOSCommand (OgreExportOptions.editXMLconverter.text + " \"" + editFilename.text + ".skeleton.xml\" \"" + editFilename.text + ".skeleton\"") ;
                            DOSCommand ("copy \"" + editFilename.text + ".skeleton\" \"" + mediaPath + "\"") ;						
                        )
                        --messageBox "OgreXMLConverter has been run and files copied to the media directory."
                        print (OgreExportOptions.editXMLconverter.text + OptionConv + " \"" + editFilename.text + ".mesh.xml\" \"" + editFilename.text + ".mesh\"") ;
					)
				)
			)
		)
	) -- }}}
	
	rollout OgreExportMesh "Mesh" width:272 height:148 rolledUp:false --{{{
	(
		checkbox CBexportMesh "Export Mesh" checked:true
		checkbox CBflipnormals "Flip normals"
		checkbox CBexportUV "Export UV sets" checked:false
		checkbox CBexportColor "Export Vertex Color"
		checkbox CBexportAlpha "Export Vertex Alpha"
		spinner SPchannels "# UV channels" range:[1,8,1] type:#integer scale:1
        fn update_enabled = (
			local v = CBexportMesh.enabled and CBexportMesh.checked
			CBflipnormals.enabled = v
			CBexportColor.enabled = v
			CBexportAlpha.enabled = v
			CBexportUV.enabled = v
			SPchannels.enabled = v and CBexportUV.checked
        )
		on OgreExportMesh open do
		(
			CBexportMesh.enabled = false;

			select = getCurrentSelection() ;
			if ((select[1]!=undefined) and (iskindof select[1] GeometryClass)) then
			(
				CBexportMesh.enabled = true;
			)

            update_enabled()
		)
		on CBexportMesh changed state do update_enabled()
		on CBexportUV changed state do update_enabled()
		on SPchannels changed state do
		(
			name = OgreExportObject.selectMesh.text;
			replaceSpaces name;
			d = getNodeByName name ;
			if (classof d) == EDITABLE_POLY then
				num_channels = (polyOp.getNumMaps d) - 1;
			else
				num_channels = (meshOp.getNumMaps d) - 1;
			if (state > num_channels) then
				SPchannels.value = num_channels;
		)
	) --}}}
	
	rollout OgreExportAnimation "Animation" width:272 height:348 rolledUp:true -- {{{
	(
		checkbox CBexportSkeleton "Export Skeleton" pos:[5,4] width:150 height:19 enabled:false
		GroupBox grp3 "Export settings" pos:[4,26] width:262 height:76
		--checkbox CBbiped "Biped Export" pos:[13,44] width:246 height:19 enabled:false
		spinner SPsamplerate "Sample Rate" pos:[24,45] width:192 height:16 enabled:false range:[0,10000,0] type:#float scale:0.5 
		spinner SPiksamplerate "IK Sample Rate" pos:[24,63] width:192 height:16 enabled:false range:[0,10000,5] type:#float scale:0.5 
		checkbox CBexporthelpers "Export non-bones objects (must be in the modifier)" pos:[11,81] width:254 height:16 enabled:false checked:false
		GroupBox grp4 "Animation settings" pos:[4,106] width:261 height:229
		button addAnimation "Add" pos:[16,195] width:80 height:22 enabled:false toolTip:"add an animation to the list"
		button deleteAnimation "Delete" pos:[106,195] width:80 height:22 enabled:false toolTip:"remove an animation from the list"
		combobox ListAnimations "" pos:[16,225] width:237 height:6 enabled:false
		spinner SPframestart "start frame" pos:[32,127] width:106 height:16 enabled:false range:[0,10000,0] type:#integer scale:1
		spinner SPframeend "end frame " pos:[32,146] width:102 height:16 enabled:false range:[0,10000,100] type:#integer scale:1
		spinner SPanimlength "length (in sec)" pos:[32,166] width:130 height:16 range:[0,10000,10] type:#float scale:0.1
		--on CBbiped changed state do
		--(
		--)

		on OgreExportAnimation open do
		(
			OgreExportAnimation.CBexportSkeleton.enabled = false;
			--OgreExportAnimation.CBbiped.enabled = false;
			--OgreExportAnimation.CBbiped.checked = false;
			OgreExportAnimation.SPsamplerate.enabled = false;
			OgreExportAnimation.SPiksamplerate.enabled = false;
			OgreExportAnimation.CBexporthelpers.enabled = false;
			OgreExportAnimation.addAnimation.enabled = false;
			OgreExportAnimation.deleteAnimation.enabled = false;
			OgreExportAnimation.ListAnimations.enabled = false;
			OgreExportAnimation.SPframestart.enabled = false;
			OgreExportAnimation.SPframeend.enabled = false;
			OgreExportAnimation.SPanimlength.enabled = false;

			select = getCurrentSelection() ;
			if ((select[1]!=undefined) and (iskindof select[1] GeometryClass)) then
			(
				OgreExportAnimation.CBexportSkeleton.enabled = (getSkin(select[1]) != undefined) or (getPhysique(select[1]) != undefined) ;
				if (OgreExportAnimation.CBexportSkeleton.enabled and OgreExportAnimation.CBexportSkeleton.checked) then
				(
					--OgreExportAnimation.CBbiped.enabled = true;
					--OgreExportAnimation.CBbiped.checked = (getPhysique(select[1]) != undefined);
					OgreExportAnimation.SPsamplerate.enabled = true;
					OgreExportAnimation.SPiksamplerate.enabled = true;
					OgreExportAnimation.CBexporthelpers.enabled = true;
					OgreExportAnimation.addAnimation.enabled = true;
					OgreExportAnimation.deleteAnimation.enabled = true;
					OgreExportAnimation.ListAnimations.enabled = true;
					OgreExportAnimation.SPframestart.enabled = true;
					OgreExportAnimation.SPframeend.enabled = true;
					OgreExportAnimation.SPanimlength.enabled = true;
				)
			)
			
			Anims = exportAnims names:#() startframes:#() endframes:#() lengths:#();
		)
		on CBexportSkeleton changed state do
		(
			select = getCurrentSelection() ;
			if ((select[1]!=undefined) and (iskindof select[1] GeometryClass)) then
			(
				--OgreExportAnimation.CBbiped.enabled = state;
				--OgreExportAnimation.CBbiped.checked = (getPhysique(select[1]) != undefined);
				OgreExportAnimation.SPsamplerate.enabled = state;
				OgreExportAnimation.SPiksamplerate.enabled = state;
				OgreExportAnimation.CBexporthelpers.enabled = state;
				OgreExportAnimation.addAnimation.enabled = state;
				OgreExportAnimation.deleteAnimation.enabled = state;
				OgreExportAnimation.ListAnimations.enabled = state;
				OgreExportAnimation.SPframestart.enabled = state;
				OgreExportAnimation.SPframeend.enabled = state;
				OgreExportAnimation.SPanimlength.enabled = state;
			)
		)
		on addAnimation pressed do
		(
			if (ListAnimations.text != "") then
			(
				local array=#();
				for n in ListAnimations.items do
					append array n;
				append array ListAnimations.text;
				ListAnimations.items = array;
				append Anims.names ListAnimations.items[ListAnimations.items.count];
				append Anims.startframes SPframestart.value;
				append Anims.endframes SPframeend.value;
				append Anims.lengths SPanimlength.value;
				ListAnimations.selection = 0;
			)		
		)
		on deleteAnimation pressed do
		(
			if ((ListAnimations.items.count > 0) and (ListAnimations.selection > 0)) then
			(
				local array=#();
				ind = ListAnimations.selection
				deleteItem Anims.names ind
				deleteItem Anims.startframes ind
				deleteItem Anims.endframes ind
				deleteItem Anims.lengths ind
				for n in Anims.names do
					append array n;
				ListAnimations.items = array;
				ListAnimations.selection = 1;
			)
		)
		on ListAnimations selected num  do
		(
			if (ListAnimations.items.count >= num) then
			(
				SPframestart.value = Anims.startframes[num]
				SPframeend.value = Anims.endframes[num]
				SPanimlength.value = Anims.lengths[num]
			)		
		)
		on SPframestart changed val do
		(
			if ((ListAnimations.items.count > 0) and (ListAnimations.selection > 0)) then
			(
				if (ListAnimations.text == Anims.names[ListAnimations.selection]) then
					Anims.startframes[ListAnimations.selection] = SPframestart.value;
			)		
		)
		on SPframeend changed val do
		(
			if ((ListAnimations.items.count > 0) and (ListAnimations.selection > 0)) then
			(
				if (ListAnimations.text == Anims.names[ListAnimations.selection]) then
					Anims.endframes[ListAnimations.selection] = SPframeend.value;
			)		
		)
		on SPanimlength changed val do
		(
			if ((ListAnimations.items.count > 0) and (ListAnimations.selection > 0)) then
			(
				if (ListAnimations.text == Anims.names[ListAnimations.selection]) then
					Anims.lengths[ListAnimations.selection] = SPanimlength.value;
			)		
		)
	) -- }}}
	
	rollout OgreExportOptions "Options" width:272 height:140 rolledUp:false -- {{{
	(
		button openScript "open OgreScript.ini" pos:[7,8] width:116 height:26 toolTip:"open the script to edit your settings"
		checkbox CBconvertXML "convert XML file after export" pos:[8,42] width:255 height:22 checked:true
		label lbl01 "XML Converter Program" pos:[8,64] width:256 height:18
		editText editXMLconverter "" pos:[7,80] width:253 height:23
		button browseXMLconverter "Browse" pos:[188,106] width:72 height:25 toolTip:"choose your XML converter"
		groupBox grp1 "Conversion settings" pos:[8,132] width:254 height:125
		checkbox CBgenerateedges "Generate Edges List (for stencil shadows)" pos:[12,145] width:245 height:20 checked:false
		checkbox CBgeneratetangents "Generate Tangent Vectors (for normal mapping)" pos:[12,163] width:245 height:20 checked:false
		checkbox CBgenerateLOD "Generate LOD" pos:[12,181] width:245 height:20 checked:false
		label lbl02 "LOD levels" pos:[30,200] width:156 height:18
		label lbl03 "LOD distance" pos:[30,218] width:156 height:18
		label lbl04 "LOD reduction (%)" pos:[30,236] width:156 height:18
		spinner SPlodlevels "" pos:[120,200] width:114 height:16 range:[1,20,1] type:#integer scale:1 enabled:false
		spinner SPloddistance "" pos:[120,218] width:114 height:16 range:[0.0,100000.0,10.0] type:#float scale:1.0 enabled:false
		spinner SPlodreduction "" pos:[120,236] width:114 height:16 range:[0.0,100.0,20.0] type:#float scale:5.0 enabled:false

		on CBgenerateLOD changed state do
		(
			SPlodlevels.enabled = state;
			SPloddistance.enabled = state;
			SPlodreduction.enabled = state;
		)
		on openScript pressed  do
		(
			shellLaunch ((getDir #scripts) + "\\ogre\\ogreScript.ini") "" ;
		)
		on CBconvertXML changed state do
		(
			if (state and (editXMLconverter.text!="")) then
			(
				if (not (doesFileExist editXMLconverter.text)) then
				(
					editXMLconverter.text = "The file/directory specified in the .ini for the XML converter does not exist !";
					CBconvertXML.checked = false;
				)
			)
		)
		on browseXMLconverter pressed  do
		(
			filename = getOpenFileName types:"Executables(*.exe)|*.exe|" ;			
			if (filename != undefined) then
			(
				editXMLconverter.text = filename ;
				CBconvertXML.checked = true;
				if (not (doesFileExist editXMLconverter.text)) then
				(
					editXMLconverter.text = "The file/directory specified in the .ini for the XML converter does not exist !";
					CBconvertXML.checked = false;
				)
				else
				(
					xmlConvPath = getFilenamePath editXMLconverter.text;
					xmlexe = getFilenameFile editXMLconverter.text;
					setINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Directories" "XMLConverterPath" xmlConvPath;
					setINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Exe" "XMLConverterExe" xmlexe;
				)
			)
		)
		on OgreExportOptions open  do
		(
			runXMLConverter = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Tools" "runXMLConverter"
			if (runXMLConverter=="yes") then
			(
				CBconvertXML.enabled = true;
				CBconvertXML.checked = true;
			)
			else
			(
				CBconvertXML.enabled = false;
			)
			
			xmlConvPath = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Directories" "XMLConverterPath"
			xmlexe = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Exe" "XMLConverterExe"
			
			ext = substring xmlexe (xmlexe.count-4) 4;
			if ( (ext[2]!="e" and ext[2]!="E") or (ext[3]!="x" and ext[3]!="X") or (ext[4]!="e" and ext[4]!="E") ) then
				editXMLconverter.text = xmlConvPath + "\\" + xmlexe + ".exe";
			else
				editXMLconverter.text = xmlConvPath + "\\" + xmlexe;

			print editXMLconverter.text;
			if (not (doesFileExist editXMLconverter.text)) then
			(
				editXMLconverter.text = "The file/directory specified in the .ini for the XML converter does not exist !";
				CBconvertXML.checked = false;
			)
			else
			(
				CBconvertXML.checked = true;
			)
		)
	) -- }}}

	rollout OgreExportAbout "About" width:272 height:48 rolledUp:true -- {{{
	(
		label labelmy1 "[ ORIGINALS ]" align:#center;
		label label11 "For use with the Ogre graphics engine." align:#left;
		label label12 "Website: http://ogre.sourceforge.net" align:#left;
		label label13 "Distributed under the terms of the LGPL." align:#left ;
		label label16 "Based on the exporter realised by" align:#left ;
		label label14 "EarthquakeProof-mallard@iie.cnam.fr(summer 2003)" align:#left ;
		label label15 "by Banania - November 2004" align:#left ;
		label labelmy2 "[ EDITING ]" align:#center;
		label labelmy3 "Further editing by Jitin Sameer" align:#left;
		label labelmy4 "- edit 1 (Jan 2010): restructured UI, UI events" align:#left;
		label labelmy5 "- edit 2 (Jan 2010): enabled texture image copy" align:#left;
		label labelmy6 "- edit 3 (Jan 2010): better MeshTools usage" align:#left;
		label labelmy7 "Heavily simplified/reworked for Grit by Dave Cunningham" align:#left;
	) -- }}}
	
	-- create the floater
	OgreExportFloater = newRolloutFloater "Grit Mesh Exporter" 280 600 ;
	addRollout OgreExportObject OgreExportFloater ;
	addRollout OgreExportMesh OgreExportFloater ;
	addRollout OgreExportAnimation OgreExportFloater ;
	addRollout OgreExportOptions OgreExportFloater ;
	addRollout OgreExportAbout OgreExportFloater ;
)
