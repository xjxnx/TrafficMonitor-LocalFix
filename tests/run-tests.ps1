$ErrorActionPreference = 'Stop'
$testRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$sourceRoot = Split-Path -Parent $testRoot
$outRoot = Join-Path $sourceRoot 'Bin\x64\Release'
if (!(Get-Command cl.exe -ErrorAction SilentlyContinue)) { throw 'Run from a Visual Studio x64 Developer PowerShell with C++/CLI and MFC installed.' }
function Invoke-CheckedTool([string]$tool, [string[]]$toolArgs) {
    & $tool @toolArgs
    if ($LASTEXITCODE -ne 0) { throw "$tool failed with exit code $LASTEXITCODE" }
}
Push-Location $outRoot
try {
    Invoke-CheckedTool 'cl.exe' @('/nologo','/EHsc','/MD','/std:c++17', (Join-Path $testRoot 'native_regressions.cpp'), '/Fe:native_regressions.exe','/Fo:native_regressions.obj','/link','user32.lib')
    Invoke-CheckedTool '.\native_regressions.exe' @()
    Invoke-CheckedTool 'cl.exe' @('/nologo','/EHsc','/MD','/std:c++17',('/I'+(Join-Path $sourceRoot 'include')),(Join-Path $testRoot 'hardware_smoke.cpp'),'/Fe:hardware_smoke.exe','/Fo:hardware_smoke.obj','/link','OpenHardwareMonitorApi.lib')
    Invoke-CheckedTool '.\hardware_smoke.exe' @()
    $cscPath = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
    Invoke-CheckedTool $cscPath @('/nologo','/target:library','/out:FakeHardware.dll','/reference:LibreHardwareMonitorLib.dll',(Join-Path $testRoot 'FakeHardware.cs'))
    Invoke-CheckedTool 'cl.exe' @('/nologo','/clr','/EHa','/MD','/DOPENHARDWAREMONITOR_EXPORTS',('/I'+(Join-Path $sourceRoot 'include')),'/FULibreHardwareMonitorLib.dll','/FUFakeHardware.dll',(Join-Path $testRoot 'sensor_regressions.cpp'),(Join-Path $sourceRoot 'OpenHardwareMonitorApi\OpenHardwareMonitorImp.cpp'),(Join-Path $sourceRoot 'OpenHardwareMonitorApi\UpdateVisitor.cpp'),'/Fe:sensor_regressions.exe','/Fo:.\')
    Invoke-CheckedTool '.\sensor_regressions.exe' @()
} finally { Pop-Location }
