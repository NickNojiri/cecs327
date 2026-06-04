# This script opens 4 separate, interactive PowerShell windows running the echo client.

# Ensure the executable exists
if (-not (Test-Path "Sockets/echo-client/build-msys/echo_client.exe")) {
    Write-Host "echo_client.exe not found! Compiling it now..." -ForegroundColor Yellow
    $env:PATH = "C:\msys64\usr\bin;" + $env:PATH
    mkdir -Force Sockets/echo-client/build-msys
    g++ -std=c++20 Sockets/echo-client/main.cpp -o Sockets/echo-client/build-msys/echo_client.exe
}

Write-Host "Launching 4 clients in separate PowerShell windows..." -ForegroundColor Green

for ($i = 1; $i -le 4; $i++) {
    Start-Process powershell -ArgumentList "-NoExit", "-Command", "`$env:PATH='C:\msys64\usr\bin;' + `$env:PATH; `$Host.UI.RawUI.WindowTitle='Client $i'; ./Sockets/echo-client/build-msys/echo_client.exe"
}
