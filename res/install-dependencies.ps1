. "$PSScriptRoot/common.ps1"

# Filter for Expand-Zip to only unpack exe and dlls
$exeAndDllFilter = ".*\.(exe|dll)"

# A temporary path we can use for downloads
$tempFilePath = [System.IO.Path]::GetTempFileName()

# Install winresourcer
npm install -g winresourcer

# Fetch PETool
Invoke-WebRequest "http://wiz0u.free.fr/prog/PETool/latest.php" -OutFile $tempFilePath
Expand-Zip $tempFilePath $PSScriptRoot $exeAndDllFilter
Remove-Item -Force $tempFilePath

# Fetch verpatch
Invoke-WebRequest "https://github.com/pavel-a/ddverpatch/files/1215010/verpatch-1.0.15.1-x86-codeplex.zip" -MaximumRedirection 10 -OutFile $tempFilePath
Expand-Zip $tempFilePath $PSScriptRoot $exeAndDllFilter
Remove-Item -Force $tempFilePath
