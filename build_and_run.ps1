D:\Dev\remedybg\remedybg.exe stop-debugging

$stopwatch = [Diagnostics.Stopwatch]::StartNew()

Push-Location bin
..\build.bat
Pop-Location

$stopwatch.Stop()

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compile time:"$stopwatch.ElapsedMilliseconds"ms"
    D:\Dev\remedybg\remedybg.exe start-debugging
}
else {
    Write-Host "Compilation failed."
}

