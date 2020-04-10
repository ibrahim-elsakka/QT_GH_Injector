# Qt GH DLL Injector
- Rebuild Guided Hacking DLL Injector
- Download link can be found on the Release

## Picture
![alt_text](https://raw.githubusercontent.com/multikill/_gui_injector/master/_gui_injector/Injector_Screenshot.png)

## How to build
- Download https://www.qt.io/download-qt-installer
- Download https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools-19123
- MSVC -> Extensions -> Qt VS Tools -> Qt Options -> Add -> SELECT YOUR DIRECTORY
- MSVC Project Settings -> Qt Project Settings -> Qt Installtion -> SELECT YOUR VERSION
- Restart MSVC to repair the intellisense

## Feature:
- Commandline interface
- Hotkeys


## ToDo:
- Sort pid by number not alphabetical
- Recode commandline interface
- update readme picture
- Recode unzip

## Bugs:
-

## Rejected:
- No auto update on x86 version, only update check 
-- Solution 1: need to compile x86 version with open ssl 
-- Solution 2: Use URLDownloadToFile instead of QT download manager


## Credits:
- Broihon
- https://guidedhacking.com/resources/guided-hacking-dll-injector.4/
- https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle
- https://www.codeproject.com/Articles/280650/Zip-Unzip-using-Windows-Shell
- https://github.com/jarro2783/cxxopts

## License
All original licenses of all used components Qt are respected with the additional exception that compiling, linking or using is allowed. Go to Qt website and check for License.

