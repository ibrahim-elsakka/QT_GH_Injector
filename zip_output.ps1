# Execute Script
# 1. Run Powershell as Admin
# 2. set-executionpolicy remotesigned

Remove-Item .\Out -Recurse
Remove-Item QT_GH_Injector.zip
New-Item -Name "Out" -ItemType "directory"

copy .\x64\Static\QT_GH_Injector.exe .\Out\GH_Injector_x64.exe
copy .\Win32\Static\QT_GH_Injector.exe .\Out\GH_Injector_x86.exe
copy .\Injector_dll\*.dll .\Out\
Compress-Archive .\Out\* QT_GH_Injector.zip