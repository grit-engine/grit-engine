SET PATH=C:\Program Files (x86)\Google\Cloud SDK\google-cloud-sdk\bin;%PATH%;
CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat"
git pull --recurse-submodules
git submodule update
msbuild /p:Configuration=Normal

CALL gsutil cp Normal/engine/engine.exe gs://grit-engine/
CALL gsutil cp Normal/launcher/launcher.exe gs://grit-engine/
CALL gsutil cp Normal/extract/extract.exe gs://grit-engine/
CALL gsutil cp Normal/GritXMLConverter/GritXMLConverter.exe gs://grit-engine/

exit
