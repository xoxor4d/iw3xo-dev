## IW3xo - A Call of Duty 4 Modification
This project is aimed at developers and includes various modifications/additions.  
IW3xo is not compatible with CoD4x, so make sure you do have a stock non-steam 1.7 CoD4.  
A compatible Radiant, built for use with IW3xo, that enables a live-link between CoD4 and Radiant can be found below.

#### Building the project:
1. Hit "generate.bat" to build project files with premake
2. Load the project in Visual Studio and open project->settings to setup paths (Debug/Release):
   - General: output directory path -> "path-to-cod4-root\"
   - Debugging: command -> "path-to-cod4-root\iw3xo.exe"
   - Debugging: working directory -> "path-to-cod4-root\"
   - You may need to change the Toolset of both glm and iw3x if you do not have v142 installed. For each go to General -> Platform Toolset -> set to your installed version
3. Build -> Build Solution
   
#### Generating the exe:   
1. Copy a non-steam (1.7) "iw3mp.exe" into the "\assets\" folder
2. Install required resource tools by running "\res\res-tools.ps1"
3. "\res\generate-exe.ps1" will generate and place "iw3xo.exe" into the project root
4. Copy the generated exe into your cod4-root
5. You only need to do this once

Requires:
1. https://github.com/xoxor4d/xcommon_iw3xo
2. https://github.com/xoxor4d/xcommon_iw3xo_menu

Optional:
- https://github.com/xoxor4d/iw3xo-radiant

## Credits
- The IW4x Team (client base)
- The Plutonium Project Team

## Disclaimer
This software has been created purely for the purposes of academic research. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.
