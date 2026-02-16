param(
    [string]$Port
)

if (-not $Port) {
    Write-Host "Enter serial port for upload (e.g. COM3). Leave empty to let PlatformIO auto-detect:" -NoNewline
    $Port = Read-Host " "
}

if ([string]::IsNullOrWhiteSpace($Port)) {
    Write-Host "Uploading (PlatformIO auto-detecting port)..."
    py -3 -m platformio run -e esp32dev -t upload
    if ($LASTEXITCODE -eq 0) { py -3 -m platformio device monitor -b 115200 }
} else {
    Write-Host "Uploading to port $Port..."
    py -3 -m platformio run -e esp32dev -t upload --upload-port $Port
    if ($LASTEXITCODE -eq 0) { py -3 -m platformio device monitor -b 115200 -p $Port }
}
