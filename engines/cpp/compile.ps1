param (
    [switch]$Clean,
    [switch]$Test
)

# 1. Clean Method
if ($Clean -and (Test-Path build)) {
    Write-Host "--- Cleaning build directory ---" -ForegroundColor Cyan
    Remove-Item -Recurse -Force build
}

# 2. Configure (Modern -S and -B approach)
Write-Host "--- Configuring Project ---" -ForegroundColor Magenta
cmake -S . -B build `
  -DCMAKE_TOOLCHAIN_FILE="$env:LOCALAPPDATA/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -G "Ninja"

# 3. Build
Write-Host "--- Building Project ---" -ForegroundColor Magenta
cmake --build build

# Check if build was successful ($LASTEXITCODE is a PowerShell variable for the last command's status)
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed! Skipping tests." -ForegroundColor Red
    exit $LASTEXITCODE
}

# 4. Test Method
if ($Test) {
    Write-Host "--- Running Test ---" -ForegroundColor Green
    
    # Define paths relative to the root where the script is run
    $EXE = "build\bin\main.exe"
    $INPUT = "..\..\tests\example\input.html"
    $YAML = "..\..\tests\example\example.xsee.yaml"

    if (Test-Path $EXE) {
        # Running the command exactly as requested
        & $EXE $INPUT --yaml $YAML
    } else {
        Write-Host "Error: main.exe not found in build folder." -ForegroundColor Red
    }
}