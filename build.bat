@echo off

title Compilando ARDiscordBypass v2.0

echo.
echo =========================================================
echo       Compilando ARDiscordBypass v2.0 (C++)
echo =========================================================
echo.

if exist "ARDiscordBypass.exe" del /f "ARDiscordBypass.exe"

where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo ERRO: g++ nao encontrado no PATH!
    echo Instale o MinGW: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo Compilando com linkagem estatica...
g++ -O2 -std=c++20 -static main.cpp -lws2_32 -lwinhttp -lshell32 -o ARDiscordBypass.exe

if %errorlevel% neq 0 (
    echo.
    echo ERRO: Falha na compilacao!
    pause
    exit /b 1
)

if exist "ARDiscordBypass.exe" (
    echo.
    echo =========================================================
    echo  Compilacao concluida com sucesso!
    echo  Arquivo: ARDiscordBypass.exe
    echo  (Linkagem estatica - sem dependencia de DLLs)
    echo =========================================================
)
pause
