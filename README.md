
<h1 align="center">iw3xo - a Call of Duty 4 modification</h3>

<p align="center">
This project is aimed at developers and includes various modifications/additions.  
iw3xo is not compatible with CoD4x, so make sure you do have a stock non-steam 1.7 CoD4.  
A compatible Radiant, built for use with IW3xo, that enables a live-link between CoD4 and Radiant can be found below.
</p>

<br>
<div align="center" markdown="1">

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/xoxor4d/iw3xo-dev?color=%2368BC71&logo=github)](https://github.com/xoxor4d/iw3xo-dev/releases)&ensp;
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/xoxor4d/iw3xo-dev/latest/develop?logo=github)&ensp;
[![Downloads](https://img.shields.io/github/downloads/xoxor4d/iw3xo-dev/total?logo=github&label=total-downloads)](https://github.com/xoxor4d/iw3xo-dev/releases)&ensp;
[![Discord](https://img.shields.io/discord/677574256678141973?label=Discord&logo=discord&logoColor=%23FFFF&)](https://discord.gg/t5jRGbj)&ensp;

<br>

### nightly builds - develop branch
( download and install the [latest release](https://github.com/xoxor4d/iw3xo-dev/releases) before using nightly's )

[![build-develop](https://img.shields.io/github/actions/workflow/status/xoxor4d/iw3xo-dev/build-debug.yml?branch=develop&label=nightly-debug&logo=github)](https://nightly.link/xoxor4d/iw3xo-dev/workflows/build-debug/develop/Debug%20binaries.zip)&ensp;
[![build-release](https://img.shields.io/github/actions/workflow/status/xoxor4d/iw3xo-dev/build-release.yml?branch=develop&label=nightly-release&logo=github)](https://nightly.link/xoxor4d/iw3xo-dev/workflows/build-release/develop/Release%20binaries.zip)&ensp;

<br>

### > Features / Guides / In-Depth <
https://xoxor4d.github.io/projects/iw3xo/

</div>

<br>

<div align="center">
	<img src="https://xoxor4d.github.io/assets/img/iw3xo/banner.jpg"/>
	<img src="https://raw.githubusercontent.com/xoxor4d/xoxor4d.github.io/master/assets/img/daynight/small_gif.gif"/>
</div>

<br>
<br>

___
## Installation

<br>

1. Download the latest [release](https://github.com/xoxor4d/iw3xo-dev/releases)
2. Copy the `.zip` contents into your cod4 root folder
3. Start __iw3xo.exe__

- [NIGHTLY]&ensp; unpack and replace `iw3x.dll` with the one found in your cod4 root folder

<br>

## nvidia-remix-branch (rtx)
1. All of the above

2. Open [Github Actions](https://github.com/xoxor4d/iw3xo-dev/actions), select the latest `rtx` branch build, grab the `Release-binaries-rtx` artifacts and extract it into your cod4 root folder.

2. Grab files from `assets-remix` and put them into your cod4 root folder (download repo as zip) 

3. Open the in-game console and type `/exec rtx` to setup dvars  

4. Restart the game

<br>

- Disable all culling: use commandline argument `-disable_culling`.  
eg: &ensp;`"c:\path\iw3xo.exe" -disable_culling` 

- Additional settings (in-game): use console command `/devgui` and go to the __rtx__ tab

- Use dvar `r_forceLod` to force a specific LOD
- Use dvar `r_forceLod_second_lowest` to hinder the game from using the lowest possible LOD (lowest LOD is sometimes used to hide the model)
- Use dvar `r_lodScaleRigid` to adjust model LOD distance - a value of 0 will force LOD0
- Use dvar `rtx_hacks` to replace the skybox with a blue-gray texture

<br>

### General tips

- You might need to assign the sky category manually (if your map is black).   
Open remix -> Go to the game setup tab -> Step 1 -> Open Sky Texture -> Use your mouse to select the sky (in the world) and assign the sky category

- If you want to use a custom skysphere (found in devgui - rtx tab), you need to assign the __Ignore Texture__ category to the blue-gray sky (mentioned one bullet point above). Then spawn the skysphere using the devgui. If the sky remains black, assign the sky category to the _new_ sky (like mentioned in the bullet point above). You only need to do this once if you save your settings afterwards.

- Spawning lights with the devgui should be pretty self explanatory. Just note that lights only update (color, radius) when you change the position afterwards.

<br>

___
## Requirements:

<br>

1. CoD4 1.7
2. https://github.com/xoxor4d/xcommon_iw3xo (compiled, placed into root/zone/english/)
3. https://github.com/xoxor4d/xcommon_iw3xo_menu (compiled, placed into root/zone/english/)
4. English localization (other languages work when renamed:)
> - __root/localization.txt__ &ensp; -> change first line to "english"
> - __root/main/__ &ensp; -> rename "localized_yourlanguage_iw**.iwd" files to "localized_english_iw**.iwd"
> - __root/zone/__ &ensp; -> rename folder "yourlanguage" to "english"

<br>

___
## Build / Compile

<br>

### > How to build / compile the project using Visual Studio
1. Clone the repo! __(zip does not include deps!)__
2. Use __generate-buildfiles.bat__ to build project files with premake
3. Load the solution `(build/iw3xo-dev.sln)` and open the iw3x project-settings to setup paths:
> - General &ensp; &ensp; &ensp; output directory path -> `path-to-cod4-root\`
> - Debugging &ensp; command -> `path-to-cod4-root\iw3xo.exe`
> - Debugging &ensp; working directory -> `path-to-cod4-root`
4. Build -> Build Solution or run with debugger

<br>

___

<br>

Optional:  
https://github.com/xoxor4d/iw3xo-radiant

Project Page:  
https://xoxor4d.github.io/projects/iw3xo/

Discord:  
https://discord.gg/t5jRGbj

<br>

## Credits
- [X Labs](https://github.com/XLabsProject) (especially [Snake](https://github.com/momo5502))
- [The Plutonium Project Team](https://plutonium.pw/) (especially [Rektinator](https://github.com/RektInator))
- [Nukem9 - LinkerMod](https://github.com/Nukem9/LinkerMod)
- [g-truc - glm](https://github.com/g-truc/glm/)
- [ocornut - Dear ImGui](https://github.com/ocornut/imgui)
- [Jelvan1 - cgame proxymod](https://github.com/Jelvan1/cgame_proxymod)

<br>

___
## Images

<br>

<img src="https://xoxor4d.github.io/assets/img/iw3xo/collisionClip.jpg"/>
<img src="https://raw.githubusercontent.com/xoxor4d/xoxor4d.github.io/master/assets/img/iw3xo/mainmenu.jpg"/>
<img src="https://raw.githubusercontent.com/xoxor4d/xoxor4d.github.io/master/assets/img/iw3xo/feat_spmap.jpg"/>

<br>

## Disclaimer
This software has been created purely for the purposes of academic research. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.
