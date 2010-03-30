fileIn (grit_exporter_dir+"util.ms")

-- global structures
struct exportOptions (sampleRate, ikSampleRate, exportAmbient, exportAlpha, blended, exportHelpers)
struct exportAnims (names, startframes, endframes, lengths)

global Anims
global Options

global ini_file_name = "grit.ini"

global GritRolloutOgreMain

fileIn (grit_exporter_dir+"mesh.ms")
fileIn (grit_exporter_dir+"skeleton.ms")
fileIn (grit_exporter_dir+"biped.ms")

rollout GritRolloutOgreMesh "Mesh" width:272 rolledUp:false --{{{
(
    checkbox CBexportMesh "Export Mesh" checked:true
    spinner SPblended "Blended Textures" range:[0,8,1] type:#integer scale:1
    checkbox CBexportColor "Export Vertex Ambient Lighting"
    checkbox CBexportAlpha "Export Vertex Alpha"
    checkbox CBgeneratetangents "Will have normal map" checked:false

    fn update_enabled = (
        local v = CBexportMesh.enabled and CBexportMesh.checked
        SPblended.enabled = v
        CBexportColor.enabled = v
        CBexportAlpha.enabled = v
        CBgeneratetangents.enabled = v
    )
    on GritRolloutOgreMesh open do update_enabled()
    on CBexportMesh changed state do update_enabled()
) --}}}

rollout GritRolloutOgreAnimation "Animation" width:272 rolledUp:true -- {{{
(
    checkbox CBexportSkeleton "Export Skeleton" enabled:false
    --checkbox CBbiped "Biped Export" pos:[13,44] width:246 height:19 enabled:false
    spinner SPsamplerate "Sample Rate" enabled:false range:[0,10000,0] type:#float scale:0.5 
    spinner SPiksamplerate "IK Sample Rate" enabled:false range:[0,10000,5] type:#float scale:0.5 
    checkbox CBexporthelpers "Export non-bones objects (must be in the modifier)" enabled:false checked:false
    button addAnimation "Add" enabled:false toolTip:"add an animation to the list"
    button deleteAnimation "Delete" enabled:false toolTip:"remove an animation from the list"
    combobox ListAnimations "" enabled:false
    spinner SPframestart "start frame" enabled:false range:[0,10000,0] type:#integer scale:1
    spinner SPframeend "end frame " enabled:false range:[0,10000,100] type:#integer scale:1
    spinner SPanimlength "length (in sec)" range:[0,10000,10] type:#float scale:0.1
    --on CBbiped changed state do
    --(
    --)

    on GritRolloutOgreAnimation open do
    (
        CBexportSkeleton.enabled = false;
        --CBbiped.enabled = false;
        --CBbiped.checked = false;
        SPsamplerate.enabled = false;
        SPiksamplerate.enabled = false;
        CBexporthelpers.enabled = false;
        addAnimation.enabled = false;
        deleteAnimation.enabled = false;
        ListAnimations.enabled = false;
        SPframestart.enabled = false;
        SPframeend.enabled = false;
        SPanimlength.enabled = false;

        select = getCurrentSelection()
        if ((select[1]!=undefined) and (iskindof select[1] GeometryClass)) then
        (
            CBexportSkeleton.enabled = (getSkin(select[1]) != undefined) or (getPhysique(select[1]) != undefined)
            if (CBexportSkeleton.enabled and CBexportSkeleton.checked) then
            (
                --CBbiped.enabled = true;
                --CBbiped.checked = (getPhysique(select[1]) != undefined);
                SPsamplerate.enabled = true;
                SPiksamplerate.enabled = true;
                CBexporthelpers.enabled = true;
                addAnimation.enabled = true;
                deleteAnimation.enabled = true;
                ListAnimations.enabled = true;
                SPframestart.enabled = true;
                SPframeend.enabled = true;
                SPanimlength.enabled = true;
            )
        )
        
        Anims = exportAnims names:#() startframes:#() endframes:#() lengths:#();
    )
    on CBexportSkeleton changed state do
    (
        select = getCurrentSelection()
        if ((select[1]!=undefined) and (iskindof select[1] GeometryClass)) then
        (
            --CBbiped.enabled = state;
            --CBbiped.checked = (getPhysique(select[1]) != undefined);
            SPsamplerate.enabled = state;
            SPiksamplerate.enabled = state;
            CBexporthelpers.enabled = state;
            addAnimation.enabled = state;
            deleteAnimation.enabled = state;
            ListAnimations.enabled = state;
            SPframestart.enabled = state;
            SPframeend.enabled = state;
            SPanimlength.enabled = state;
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

rollout GritRolloutOgreMain "Main" width:272 -- {{{
(
    button chooseDir "Browse Dir" toolTip:"Choose dir where files are written" across:2
    button Export "Export Selection" toolTip:"Write OGRE files"
    edittext editDir ""
    progressBar exportProgress ""
    label lbl01 "XML Converter Program"
    editText editXMLConverter ""
    button browseXMLconverter "Browse" toolTip:"choose your XML converter"
    
    on GritRolloutOgreMain open do
    (
        local lastDir = getINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastDir"
        lastAnimName = getINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastAnimName"
        lastfirstFrame = getINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastFrameStart"
        lastlastFrame = getINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastFrameEnd"
        lastAnimLength = getINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastAnimLength"
        if lastDir != undefined then
            editDir.text = lastDir

        editXMLconverter.text = getINISetting ((getDir #scripts) + ini_file_name) "Directories" "XMLConverter"
        print editXMLconverter.text;

        if (not (doesFileExist editXMLconverter.text)) then (
            MessageBox ("Could not find OgreXMLConverter at: "+editXMLConverter.text) title:"ERROR!"
            editXMLconverter.text = ""
        )
    )
    on chooseDir pressed do
    (
        local filename = getSavePath caption:"Output directory" initialDir: editDir.text 
        if (filename != undefined) then (
            editDir.text = filename
            setINISetting ((getDir #scripts) + ini_file_name) "Settings" "lastDir" editDir.text	
        )
    )
    on browseXMLconverter pressed  do
    (
        local filename = getOpenFileName types:"Executables(*.exe)|*.exe|"
        if (filename == undefined) then return()
        editXMLConverter.text = filename

        if doesFileExist filename then
        (
            setINISetting ((getDir #scripts) + ini_file_name) "Directories" "XMLConverter" editXMLconverter.text;
        )
        else
        (
            MessageBox ("Could not find OgreXMLConverter at the given location: "+filename) title:"ERROR!"
            editXMLconverter.text = ""
        )
    )
    on Export pressed do
    (
        sliderTime = 0
        m = (getCurrentSelection())[1]
        if (editDir.text == "") then (
            messageBox "You have to choose an output directory."
        ) else if (m == undefined or not (iskindof m GeometryClass)) then (
            messageBox "You have to select some geometry for export."
        ) else (
            local out_file = editDir.text + "\\" + m.name
            clearlistener()
            
            
            local options = exportOptions exportAmbient:false exportAlpha:false blended:0
            exportingMeshDone = false
            exportingSkelDone = false
            -- sets options
            ---------------
            if (GritRolloutOgreMesh.CBexportColor.checked and GritRolloutOgreMesh.CBexportColor.enabled) then
                options.exportAmbient = true
            if (GritRolloutOgreMesh.CBexportAlpha.checked and GritRolloutOgreMesh.CBexportAlpha.enabled) then
                options.exportAlpha = true
            options.blended = GritRolloutOgreMesh.SPblended.value
            options.exportHelpers = (GritRolloutOgreAnimation.CBexporthelpers.enabled and GritRolloutOgreAnimation.CBexporthelpers.checked);
            

            -- exports mesh
            ---------------
            if (GritRolloutOgreMesh.CBexportMesh.enabled and GritRolloutOgreMesh.CBexportMesh.checked) then
            (
                --progressStart "Mesh Export"
                exportProgress.value = 0
                exportingMeshDone = write_mesh_xml m options out_file
                exportProgress.value = 0
                --progressStop
            )
        
            -- exports skeleton
            -------------------
            if (GritRolloutOgreAnimation.CBexportSkeleton.enabled and GritRolloutOgreAnimation.CBexportSkeleton.checked) then
            (
                options.sampleRate = GritRolloutOgreAnimation.SPsamplerate.value
                options.ikSampleRate = GritRolloutOgreAnimation.SPiksamplerate.value
            
                exportingSkelDone =	writeSkeleton m options Anims out_file
            )
            
            -- post traitement
            ------------------
            if (exportingMeshDone or exportingSkelDone) then 
            (					
                OptionConv = " -e";
        
                if (exportingMeshDone) then 
                (
                    if (GritRolloutOgreMesh.CBgeneratetangents.enabled and GritRolloutOgreMesh.CBgeneratetangents.checked) then
                    (
                        OptionConv = OptionConv + " -t";
                    )
                    
                    local cmd = "\"" + GritRolloutOgreMain.editXMLconverter.text + "\"" + OptionConv + " \"" + out_file + ".mesh.xml\" \"" + out_file + ".mesh\""
                    DOSCommand cmd
                    print cmd
                )
                if (exportingSkelDone) then 
                (
                    cmd = "\"" + GritRolloutOgreMain.editXMLconverter.text + "\" \"" + out_file + ".skeleton.xml\" \"" + out_file + ".skeleton\""
                    DOSCommand cmd
                    print cmd
                )
            )
        )
    )
) -- }}}

macroScript showGritOgreWindow
	category:"Grit Tools"
	internalCategory:"Grit Tools"
	buttonText:"Grit .mesh/.skeleton Exporter"
	tooltip:"Grit .mesh/.skeleton Exporter"
	Icon:#("Maintoolbar",49)
(
	
	-- create the floater
	floater = newRolloutFloater "Grit .mesh/.skeleton Exporter" 280 500
	addRollout GritRolloutOgreMain floater
	addRollout GritRolloutOgreMesh floater
	addRollout GritRolloutOgreAnimation floater
)
