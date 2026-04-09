# --- Path Initialization ---
# $PSScriptRoot is the directory containing this script.
# We go one level up to get to the project folder.
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

# --- Configuration ---
$Colors = @{
    Pass    = "Green"
    Fail    = "Red"
    Warn    = "Yellow"
    Header  = "Cyan"
    Info    = "White"
}

# These paths are now relative to the Project Root
$Engines = @{
    "Python" = { uv run engines/python/main.py $args[0] --yaml $args[1] }
    "NodeJS" = { node engines/js/index.js $args[0] --yaml $args[1] }
    "Cpp"    = { engines/cpp/build/bin/main.exe $args[0] --yaml $args[1] }
}

$Global:Results = @()
$TotalStart = Get-Date

Write-Host "`n[XSEE Test Suite] Started: $(Get-Date)" -ForegroundColor $Colors.Header
Write-Host "Project Root: $ProjectRoot" -ForegroundColor $Colors.Header
Write-Host "------------------------------------------------------------"

# Using relative path from project root
$TestDirs = Get-ChildItem -Path "tests/*" | Where-Object { $_.PSIsContainer }

foreach ($dir in $TestDirs) {
    $testName     = $dir.Name
    $InputHtml    = Join-Path $dir.FullName "input.html"
    $SchemaYaml   = Join-Path $dir.FullName "example.xsee.yaml"
    $ExpectedPath = Join-Path $dir.FullName "expected.json"

    Write-Host "`n▶ Scenario: $testName" -ForegroundColor $Colors.Header
    
    if (-not (Test-Path $ExpectedPath)) {
        Write-Host "  ! Skipping: expected.json not found." -ForegroundColor $Colors.Warn
        continue
    }

    # Normalize Expected JSON
    $ExpectedRaw = Get-Content $ExpectedPath -Raw | ConvertFrom-Json
    $ExpectedJson = $ExpectedRaw | ConvertTo-Json -Depth 10

    foreach ($engineName in $Engines.Keys) {
        $Enginestart = Get-Date
        Write-Host "  [~] $engineName : " -NoNewline
        
        try {
            # Execute and capture BOTH stdout and stderr
            # Note: The & operator executes the script block defined in $Engines
            $RawOutput = & $Engines[$engineName] $InputHtml $SchemaYaml 2>&1
            
            $Duration = [Math]::Round(((Get-Date) - $Enginestart).TotalMilliseconds, 2)

            # Check if output is an Error Record or if exit code is non-zero
            if ($RawOutput -is [System.Management.Automation.ErrorRecord] -or $LASTEXITCODE -ne 0) {
                Write-Host "DRIVER ERROR" -ForegroundColor $Colors.Fail
                Write-Host "      $RawOutput" -ForegroundColor $Colors.Warn
                $Global:Results += [PSCustomObject]@{ Test = $testName; Driver = $engineName; Status = "ERROR"; Time = "$Duration ms" }
                continue
            }

            $ActualRaw = $RawOutput | Out-String | ConvertFrom-Json
            $ActualJson = $ActualRaw | ConvertTo-Json -Depth 10

            if ($ActualJson -eq $ExpectedJson) {
                Write-Host "PASS" -ForegroundColor $Colors.Pass -NoNewline
                Write-Host " ($Duration ms)" -ForegroundColor $Colors.Info
                $Global:Results += [PSCustomObject]@{ Test = $testName; Driver = $engineName; Status = "PASS"; Time = "$Duration ms" }
            } else {
                Write-Host "FAIL" -ForegroundColor $Colors.Fail -NoNewline
                Write-Host " ($Duration ms)" -ForegroundColor $Colors.Info
                
                Write-Host "      --- DIFF DETECTED ---" -ForegroundColor $Colors.Fail
                Write-Host "      EXPECTED:" -ForegroundColor $Colors.Pass
                Write-Host "      $($ExpectedJson -replace '(?m)^', '      ')"
                Write-Host "      ACTUAL:" -ForegroundColor $Colors.Fail
                Write-Host "      $($ActualJson -replace '(?m)^', '      ')"
                
                $Global:Results += [PSCustomObject]@{ Test = $testName; Driver = $engineName; Status = "FAIL"; Time = "$Duration ms" }
            }
        } catch {
            Write-Host "RUNNER CRASH" -ForegroundColor $Colors.Fail
            Write-Host "      $($_.Exception.Message)" -ForegroundColor $Colors.Warn
            $Global:Results += [PSCustomObject]@{ Test = $testName; Driver = $engineName; Status = "CRASH"; Time = "N/A" }
        }
    }
}

# --- Final Summary ---
Write-Host "`n" + ("=" * 60) -ForegroundColor $Colors.Header
Write-Host "XSE EXECUTION SUMMARY" -ForegroundColor $Colors.Header
$Global:Results | Format-Table -AutoSize
Write-Host ("=" * 60) -ForegroundColor $Colors.Header

if ($Global:Results.Status -contains "FAIL" -or $Global:Results.Status -contains "ERROR") {
    exit 1
}