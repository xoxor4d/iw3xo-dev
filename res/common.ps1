# this is only for cmdlets, executable calls need to be checked separately!
$ErrorActionPreference = "Stop"

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

function Get-PSScriptRoot() {
    if ($PSScriptRoot -eq $null) {
        return Split-Path -Parent -Path $MyInvocation.MyCommand.Definition
    } else {
        return $PSScriptRoot
    }
}
$PSScriptRoot = Get-PSScriptRoot

function Expand-Zip($file, $destination, $filter=".*")
{
    [System.Reflection.Assembly]::LoadWithPartialName("System.IO.Compression.FileSystem") | Out-Null
    $zip = [System.IO.Compression.ZipFile]::OpenRead($file)
    $zip.Entries -match $filter | %{
        Write-Host $_
        $f = new-object -TypeName System.IO.FileInfo([System.IO.Path]::Combine($destination, $_.FullName))
        $f.Directory.Create()
        [System.IO.Compression.ZipFileExtensions]::ExtractToFile($_, $f.FullName, $true)
    }
    $zip.Dispose()
}

function Replace-Bytes(
    [string] $file,
    [byte[]] $old, 
    [byte[]] $new) {
    $in = new-object System.IO.BufferedStream ([System.IO.File]::OpenRead($file))
    $out = new-object System.IO.BufferedStream ([System.IO.File]::Create("${file}~"))
    $bufd = new-object Byte[] $old.Length
    $bufi = 0
    while (($byte = $in.ReadByte()) -ge 0) {
        if ($byte -ne $old[$bufi]) {
            # mismatch
            if ($bufi -gt 0) {
                $out.Write($bufd, 0, $bufi)
            }
            $bufi = 0
            $out.WriteByte($byte)
            continue
        }
        # match
        $bufd[$bufi] = $byte
        $bufi++
        if ($bufi -eq $old.Length) {
            # complete match
            $out.Write($new, 0, $new.Length)
            $bufi = 0
        }
    }
    $out.Flush()
    $out.Close()
    $in.Close()

    mv -Force "${file}~" $file
}

function Replace-String-KeepOffset(
    [string] $file,
    [string] $old,
    [string] $new) {
    $oldBytes = [System.Text.Encoding]::UTF8.GetBytes($old)
    $newSmallerBytes = [System.Text.Encoding]::UTF8.GetBytes($new)
    $newBytes = new-object byte[] $oldBytes.Length
    if ($newSmallerBytes.Length + 1 -lt $oldBytes.Length) {
        $start = $newSmallerBytes.Length + 1
        [System.Array]::Copy($oldBytes, $start, $newBytes, $start, $oldBytes.Length - $start)
    }
    [System.Array]::Copy($newSmallerBytes, $newBytes, $newSmallerBytes.Length)
    replace-bytes $file $oldBytes $newBytes
}