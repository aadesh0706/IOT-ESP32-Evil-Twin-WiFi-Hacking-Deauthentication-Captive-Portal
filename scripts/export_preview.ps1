param(
  [string]$Out = "assets\preview_flow.gif",
  [int]$Duration = 8,
  [int]$Fps = 15
)

Write-Host "Ensure Node.js (>=12), npm, and ffmpeg are installed and available in PATH." -ForegroundColor Yellow
Write-Host "Installing puppeteer (local, no-save) if missing..." -ForegroundColor Yellow
npm install puppeteer --no-save

Write-Host "Rendering preview to GIF..." -ForegroundColor Green
node .\scripts\render_preview.js $Out $Duration $Fps

if ($LASTEXITCODE -eq 0) { Write-Host "Preview GIF created: $Out" -ForegroundColor Green } else { Write-Host "Failed to create GIF." -ForegroundColor Red }

Write-Host "Temporary frames are left in ./tmp_preview_frames for inspection." -ForegroundColor Yellow
