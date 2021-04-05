## IW3xo - A Call of Duty 4 Modification
This project is aimed at developers and includes various modifications/additions.  
IW3xo is not compatible with CoD4x, so make sure you do have a stock non-steam 1.7 CoD4.  
A compatible Radiant, built for use with IW3xo, that enables a live-link between CoD4 and Radiant can be found below.

Pre-build binaries: https://github.com/xoxor4d/iw3xo-dev/releases  
Changelog: https://github.com/xoxor4d/iw3xo-dev/wiki/Changelog  
Project Page: https://xoxor4d.github.io/projects/iw3xo/

___

<p float="left">
  <img src="https://xoxor4d.github.io/assets/img/iw3xo/collisionClip.jpg" width="48%" />
  <img src="https://xoxor4d.github.io/assets/img/iw3xo/originVelocity.jpg" width="48%" align="right" /> 
</p>

___

#### Building the project:
1. Clone the repo (zip does not include deps)
2. Hit "generate.bat" to build project files with premake
3. Load the project in Visual Studio and open project->settings to setup paths (Debug/Release):
   - General: output directory path -> "path-to-cod4-root\\"
   - Debugging: command -> "path-to-cod4-root\iw3xo.exe"
   - Debugging: working directory -> "path-to-cod4-root\\"
   - You may need to change the Toolset of both glm and iw3x if you do not have v142 installed. For each go to General -> Platform Toolset -> set to your installed version
4. Build -> Build Solution
   
#### Generating the exe:   
1. Copy a non-steam (1.7) "iw3mp.exe" into the "\assets\\" folder
2. Install required resource tools by running "\res\res-tools.ps1"
3. "\res\generate-exe.ps1" will generate and place "iw3xo.exe" into the project root
4. Copy the generated exe into your cod4-root
5. You only need to do this once

___

Requires:
1. CoD4 1.7
2. https://github.com/xoxor4d/xcommon_iw3xo (compiled, placed into root/zone/english/)
3. https://github.com/xoxor4d/xcommon_iw3xo_menu (compiled, placed into root/zone/english/)
4. English localization (other languages work when renamed:)
  - __root/localization.txt__ -> change first line to "english"
  - __root/main/__ -> rename "localized_yourlanguage_iw**.iwd" files to "localized_english_iw**.iwd"
  - __root/zone/__ -> rename folder "yourlanguage" to "english"

___

Optional:  
https://github.com/xoxor4d/iw3xo-radiant

Project Page:  
https://xoxor4d.github.io/projects/iw3xo/

Discord:  
https://discord.gg/t5jRGbj

## Credits
- The IW4x Team (client base)
- The Plutonium Project Team

## Disclaimer
This software has been created purely for the purposes of academic research. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.
