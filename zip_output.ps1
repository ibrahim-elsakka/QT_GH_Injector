# Execute Script
# 1. Run Powershell as Admin
# 2. set-executionpolicy remotesigned

Remove-Item .\Out -Recurse
Remove-Item QT_GH_Injector.zip
New-Item -Name "Out" -ItemType "directory"

copy .\x64\Static\QT_GH_Injector.exe .\Out\GH_Injector_x64.exe
copy .\Win32\Static\QT_GH_Injector.exe .\Out\GH_Injector_x86.exe
copy .\Injector_dll\*.dll .\Out\
copy .\Injector_dll\*.exe .\Out\


# for debugging
copy .\Injector_dll\*.dll .\QT_GH_Injector\
copy .\Injector_dll\*.exe .\QT_GH_Injector\

#Compress-Archive .\Out\* QT_GH_Injector.zip