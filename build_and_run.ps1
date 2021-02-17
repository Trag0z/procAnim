D:\Dev\remedybg\remedybg.exe stop-debugging

$stopwatch = [Diagnostics.Stopwatch]::StartNew()

Push-Location bin
..\build.bat
Pop-Location

Write-Host "Compile time:"$stopwatch.ElapsedMilliseconds"ms"

$stopwatch.Stop()

D:\Dev\remedybg\remedybg.exe start-debugging