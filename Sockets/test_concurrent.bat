@echo off
:: This script opens 4 separate, interactive Command Prompt windows running the echo client.

:: Check if the executable exists
if not exist Sockets\echo-client\build-msys\echo_client.exe (
    echo echo_client.exe not found! Compiling it now...
    set PATH=C:\msys64\usr\bin;%PATH%
    if not exist Sockets\echo-client\build-msys mkdir Sockets\echo-client\build-msys
    g++ -std=c++20 Sockets\echo-client\main.cpp -o Sockets\echo-client\build-msys\echo_client.exe
)

echo Launching 4 clients in separate Command Prompt windows...

start cmd /k "title Client 1 && set PATH=C:\msys64\usr\bin;%PATH% && Sockets\echo-client\build-msys\echo_client.exe"
start cmd /k "title Client 2 && set PATH=C:\msys64\usr\bin;%PATH% && Sockets\echo-client\build-msys\echo_client.exe"
start cmd /k "title Client 3 && set PATH=C:\msys64\usr\bin;%PATH% && Sockets\echo-client\build-msys\echo_client.exe"
start cmd /k "title Client 4 && set PATH=C:\msys64\usr\bin;%PATH% && Sockets\echo-client\build-msys\echo_client.exe"
