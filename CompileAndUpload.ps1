$port = "COM6"
$fqbn = "esp32:esp32:esp32"
Write-Output "Compiling..."
arduino-cli compile --fqbn $fqbn 

if ($LASTEXITCODE -eq 0) {
    Write-Output "Upload in progress..."
    arduino-cli upload -p $port --fqbn $fqbn --verbose
    if ($LASTEXITCODE -eq 0) {
        Write-Output "Upload completed successfully."
    } else {
        Write-Output "Error: Upload failed."
    }
} else {
    Write-Output "Error: Compilation failed."
}
