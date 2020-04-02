## IW3xo - A Call of Duty 4 Modification
This project is aimed at developers and includes various modifications and additions.  
IW3xo is not compatible with CoD4x, so make sure you do have a stock non-steam 1.7 CoD4.  
A compatible Radiant, build for use with IW3xo, that enables a live-link between CoD4 <-> Radiant is available here: <url>

Build project:
1. Hit "generate.bat" to build project files
2. Load the project and open project-settings to setup paths (Debug/Release):
   - General: output directory path -> "path-to-cod4-root\"
   - Debugging: command -> "path-to-cod4-root\iw3xo.exe"
   - Debugging: working directory -> "path-to-cod4-root\"
3. Build
   
Generate the exe:   
1. Copy a non-steam (1.7) "iw3mp.exe" into the "\assets" folder
2. Install required resource tools by running "res\res-tools.ps1"
3. "res\generate-exe.ps1" will generate and place "iw3xo.exe" into the project root
4. Copy the generated exe into your cod4-root

Requires:
1. https://github.com/xoxor4d/xcommon_iw3xo
2. https://github.com/xoxor4d/xcommon_iw3xo_menu
