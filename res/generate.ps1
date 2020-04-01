. "$PSScriptRoot/common.ps1"

cp -Force "$PSScriptRoot\..\assets\iw3mp.exe" "$PSScriptRoot\iw3x.tmp"

# Remove certificate & build timestamp if any
& "$PSScriptRoot/PETool.exe" "$PSScriptRoot\iw3x.tmp"  /destamp

# Apply new version info
echo "Updating version information..."
& "$PSScriptRoot/verpatch.exe" "$PSScriptRoot\iw3x.tmp" /rpdb /langid 0x409 /va 1.7.0.0 /pv 1.7.0.0 /s copyright "No rights reserved." /s desc "IW3xo" /s title "iw3xo" /s product "IW3xo" /s OriginalFilename "iw3xo.exe" /s pb 1

# Update the icon
echo "Updating the icon..."
winresourcer --operation=delete "--exeFile=$PSScriptRoot\iw3x.tmp" --resourceType=Icongroup --resourceName=1
winresourcer --operation=add "--exeFile=$PSScriptRoot\iw3x.tmp" --resourceType=Icongroup --resourceName=1 --lang=1033 "--resourceFile=$PSScriptRoot\..\assets\ico_3xo.ico"

# Change window title -> resets stats
#echo "Updating the title..."
#replace-string-keepoffset "$PSScriptRoot\iw3x.tmp" "Call of Duty 4" "IW3xo b2939"

# Make it load iw3x.dll instead of d3d9.dll
echo "Updating the D3D9 library path..."
replace-string-keepoffset "$PSScriptRoot\iw3x.tmp" "d3d9.dll" "iw3x.dll"


# Let's place it into the root directory
mv -Force "$PSScriptRoot\iw3x.tmp" "$PSScriptRoot\..\iw3xo.exe"