# Breakout - self-update script (Windows)
# Downloads the latest release from GitHub and replaces the local copy.
# Requires: PowerShell 5.1+ (built into Windows 10/11)

$ErrorActionPreference = "Stop"

$repo = "k0fis/breakout"
$apiUrl = "https://api.github.com/repos/$repo/releases/latest"
$assetName = "breakout-windows-x86_64.zip"

Write-Host "Platform: Windows"
Write-Host "Looking for asset: $assetName"

# --- Query GitHub API --------------------------------------------------------

Write-Host "Fetching latest release info..."
try {
    $release = Invoke-RestMethod -Uri $apiUrl -UseBasicParsing
} catch {
    Write-Error "Cannot reach GitHub API. Check your network connection."
    exit 1
}

$tag = $release.tag_name
if (-not $tag) {
    Write-Error "No releases found for $repo."
    exit 1
}
Write-Host "Latest version: $tag"

# Find our asset
$asset = $release.assets | Where-Object { $_.name -eq $assetName }
if (-not $asset) {
    Write-Error "Asset '$assetName' not found in release $tag."
    exit 1
}

$downloadUrl = $asset.browser_download_url

# --- Download ----------------------------------------------------------------

$tmpDir = Join-Path $env:TEMP "breakout-update-$(Get-Random)"
New-Item -ItemType Directory -Path $tmpDir -Force | Out-Null

$zipPath = Join-Path $tmpDir $assetName

Write-Host "Downloading $assetName..."
try {
    Invoke-WebRequest -Uri $downloadUrl -OutFile $zipPath -UseBasicParsing
} catch {
    Write-Error "Download failed."
    Remove-Item -Recurse -Force $tmpDir
    exit 1
}

# --- Install -----------------------------------------------------------------

Write-Host "Extracting..."
Expand-Archive -Path $zipPath -DestinationPath "." -Force

# --- Clean up ----------------------------------------------------------------

Remove-Item -Recurse -Force $tmpDir
Write-Host "Done! Breakout updated to $tag."
