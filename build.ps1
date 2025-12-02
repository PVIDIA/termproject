$currentDir = Get-Location
if (Test-Path build) {
    Remove-Item -Recurse -Force build
}
New-Item -ItemType Directory -Path build

$vs = "C:\Program Files\Microsoft Visual Studio\2022\Community"
Import-Module "$vs\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "$vs" -DevCmdArguments "-arch=amd64 -host_arch=amd64"

Set-Location $currentDir

#$cpps = (Get-ChildItem -Path "src" -Filter *.cpp -Recurse).FullName
$cpps = @("src/main.cpp", "src/model.cpp", "src/node2.cpp", "src/game/MainScene.cpp")

cl /utf-8 $cpps /EHsc /std:c++17 /D "NDEBUG" /I .\win-x64-msvc\include\ /Fo"build\\" /Fe"build\main.exe" /link /LIBPATH:.\win-x64-msvc\lib freeglut.lib glew32.lib opengl32.lib

Copy-Item -Path ".\win-x64-msvc\bin\*" -Destination "./build" -Recurse

$env:Path = $env:Path + ";$currentDir\win-x64-msvc\bin"
Write-Host "Testbed built successfully. Running..."
.\build\main.exe
Write-Host "Testbed exited."