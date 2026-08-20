@echo off

title Compilando ARDiscordBypass C++

echo.
echo =========================================================
echo       Compilando ARDiscordBypass (Versao C++)
echo =========================================================
echo.

if exist "ARDiscordBypass.exe" del /f "ARDiscordBypass.exe"


    echo Compilando com MinGW g++...
    "C:\ProgramData\mingw64\mingw64\bin\g++.exe" -O2 -std=c++20 main.cpp -lws2_32 -lwinhttp -o ARDiscordBypass.exe
    goto :done
)

echo ERRO: Nao foi encontrado nenhum compilador C++ (g++) no seu PATH.
pause
exit /b 1

:done
if exist "ARDiscordBypass.exe" (
    echo.
    echo =========================================================
    echo Compilacao concluida com sucesso!
    echo Arquivo criado: ARDiscordBypass.exe
    echo =========================================================
    echo .
)
pause
