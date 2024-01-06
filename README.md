
<h1 align="center">iw3xo - a Call of Duty 4 modification</h3>

<p align="center">
This project is aimed at developers and includes various modifications/additions.  
iw3xo is not compatible with CoD4x, so make sure you uninstall it first.  
A compatible radiant modification that can create a live-link between iw3xo and iw3xo-radiant can be found below.
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
	<img src="https://github.com/xoxor4d/iw3xo-dev/assets/45299104/4cb3fd26-1a58-468f-9c60-49b2aba63235"/>
	<img src="https://github.com/xoxor4d/iw3xo-dev/assets/45299104/d921ead4-c86e-4385-8be3-dedb0302c690"/>
</div>

<br>
<br>

## Installation > nvidia-remix-branch (rtx) 

<div align="right" markdown="1">

[![build-release](https://img.shields.io/github/actions/workflow/status/xoxor4d/iw3xo-dev/build-rtx-release.yml?branch=rtx&label=nightly-rtx&logo=github)](https://nightly.link/xoxor4d/iw3xo-dev/workflows/build-rtx-release/rtx/Release-binaries-rtx.zip)
</div>

1. Download the latest [release](https://github.com/xoxor4d/iw3xo-dev/releases) and copy the contents of the `.zip` file into your cod4 root folder (a full release is required as it comes with additional files)

2. Open [Github Actions](https://github.com/xoxor4d/iw3xo-dev/actions) and select the latest successful `rtx` build 

3. Download the `Release-binaries-rtx` artifact. Files within `assets-remix` go into your cod4 root folder. The same goes for `iw3x.dll`. Replace all files when prompted. 

4. Run `iw3xo.exe`
> `iw3xo.exe` (modified to load `iw3x.dll` instead of `d3d9.dll`)  
> | -> `iw3x.dll` (a proxy d3d9.dll) will then load the remix runtime d3d9 proxy dll :]  
> | -> if you want to use the original `iw3mp.exe` then you have to find other ways of chain-loading the dll's 

5. Open the in-game console and type `/exec rtx` and restart the game

<br>

### Commandline Arguments:
  - `-fixed_function` &emsp; > use the fixed-function pipeline to render static models __(!RECOMMENDED)__
  - `-spawn_sky` &emsp; &emsp; &ensp; &ensp; &nbsp;> automatically spawns a fitting sky when loading a map __(!RECOMMENDED)__
  - `-thirdperson` &ensp; &ensp; &ensp; &ensp; > render thirdperson model into firstperson view (General Tips)
> eg: &ensp;`"c:\path\iw3xo.exe" -fixed_function -spawn_sky` 

<br>

### Additional Settings (console command `/devgui` - __rtx__ tab)
- I suggest binding the `/devgui` command to a key, eg: `bind F5 devgui`
- `rtx_disable_world_culling` :: disable culling (__set to `all-but-models` (3)__) - _(1 by default)_
- `rtx_disable_entity_culling` :: disable culling of script objects (game entities) - _(enabled by default)_
- `rtx_hacks` :: replace the skybox with a blue-gray texture _(enabled by default)_
- `rtx_extend_smodel_drawing` :: disable static model draw limit (max amount of static models drawn at once)
- `rtx_warm_smodels` :: build model buffers on map load _(fixed-function only - enabled by default)_
- `r_forceLod` :: force a specific LOD
- `r_forceLod_second_lowest` :: prevent game from using the lowest LOD (sometimes used to hide the model)
- `r_lodScaleRigid` :: adjust model LOD distance - a value of 0 will force LOD0


<br>

### General Tips

- You might need to assign the sky category manually (if your map is black).   
Open remix -> Go to the game setup tab -> Step 1 -> Open Sky Texture -> Use your mouse to select the sky (in the world) and assign the sky category

- If you want to use a custom skysphere (found in devgui - rtx tab), you need to assign the __Ignore Texture__ category to the blue-gray sky (mentioned one bullet point above). Then spawn the skysphere using the devgui. If the sky remains black, assign the sky category to the _new_ sky (like mentioned in the bullet point above). You only need to do this once if you save your settings afterwards.

- Swift changes in geometry (eg. teleporting) with dvar `rtx_disable_world_culling` set to `all .. (3)` can crash the game.
  Set it to `less (1)` to make it more stable. Note: World geometry hashes wont be stable! (if that matters to you)

- The above + having `r_lodScaleRigid` at a pretty low value (increases model draw distance) = more instability.

- _Game Setup-> Parameter Tuning-> Fused World-View Mode_ set to `In View Transform` fixes motion vectors but rotates the fallback distant light (this is not fixable by using a distantlight defined in your mod.usd)

- Spawning lights with the devgui should be pretty self explanatory. (8 active lights are supported)

- Using the `thirdperson` flag will render the thirdperson playermodel into the firstperson view (for shadow casting. Assign the _Player Model_ material category to hide it from your view while keeping it casting shadows. (Still WIP - also spawns a triangle with a unique texture used as the bounding box origin to hide meshes using textures marked as player model)

<br>

### Questions? 
- join the [rtx-remix showcase](https://discord.gg/j6sh7JD3v9) discord and check out the cod4 thread within the `remix-projects` channel.
- join the [iw3xo](https://discord.gg/t5jRGbj) discord if you have questions related to iw3xo
- sp variant: [iw3sp-mod-rtx](https://github.com/xoxor4d/iw3sp-mod-rtx) 
- rtx-remix: https://github.com/NVIDIAGameWorks/rtx-remix  

<br>

___
## Requirements:

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

### > How to build / compile the project using Visual Studio
1. Clone the repo! __(zip does not include deps!)__
2. Setup a `COD4_ROOT` environment variable with a path to your cod4 directory (optional)
2. Use __generate-buildfiles.bat__ to build project files with premake
3. Open `(build/iw3xo-dev.sln)` and setup paths for the iw3x project (not needed if Step #2)
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
- [Nvidia - rtx-remix](https://github.com/NVIDIAGameWorks/rtx-remix)

<br>

___
## Images

<br>

<img src="https://github.com/xoxor4d/iw3xo-dev/assets/45299104/8fca51c8-3642-4ddf-84d9-9324e7798093"/>
<img src="https://github.com/xoxor4d/iw3xo-dev/assets/45299104/edf972dc-0a63-4d3d-8bfb-4987c3dc6115"/>

<br>
<br>

## Disclaimer
This software has been created purely for the purposes of academic research. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.
