param(
    [string]$Port
)

if (-not $Port) {
    Write-Host "Enter serial port (e.g. COM3). Leave empty to let PlatformIO auto-detect and upload:" -NoNewline
    $Port = Read-Host " "
}

if ([string]::IsNullOrWhiteSpace($Port)) {
    Write-Host "Uploading (PlatformIO auto-detecting port)..."
    py -3 -m platformio run -e esp32dev -t upload
} else {
    Write-Host "Uploading to port $Port..."
    py -3 -m platformio run -e esp32dev -t upload --upload-port $Port
}
