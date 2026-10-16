@echo off

title Compilando ARDiscordBypass v3.5.0

echo.
echo =========================================================
echo       Compilando ARDiscordBypass v3.5.0 (C++)
echo =========================================================
echo.

set BUILD_GUI=0
if "%1"=="--gui" set BUILD_GUI=1
if "%1"=="/gui" set BUILD_GUI=1

where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo ERRO: g++ nao encontrado no PATH!
    echo Instale o MinGW-w64: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo [1/3] Embutindo proton-confgen.exe...
cd bin
objcopy -I binary -O pe-x86-64 -B i386:x86-64 proton-confgen.exe ..\proton_confgen_embed.o
cd ..

echo [2/3] Compilando CLI: ARDiscordBypass.exe...
g++ -O3 -std=c++20 -static -static-libgcc -static-libstdc++ main.cpp proton_confgen_embed.o -lws2_32 -lwinhttp -o ARDiscordBypass.exe
strip ARDiscordBypass.exe 2>nul

if %errorlevel% neq 0 (
    echo ERRO: Falha na compilacao do CLI!
    pause
    exit /b 1
)

if "%BUILD_GUI%"=="1" (
    echo [3/3] Compilando GUI Opcional: ARDiscordBypass_gui.exe...
    g++ -O3 -std=c++20 -static -static-libgcc -static-libstdc++ ^
        gui/main.cpp ^
        gui/app_window.cpp ^
        proton_confgen_embed.o ^
        gui/vendor/imgui/imgui.cpp ^
        gui/vendor/imgui/imgui_draw.cpp ^
        gui/vendor/imgui/imgui_tables.cpp ^
        gui/vendor/imgui/imgui_widgets.cpp ^
        gui/vendor/imgui/backends/imgui_impl_win32.cpp ^
        gui/vendor/imgui/backends/imgui_impl_opengl3.cpp ^
        -I . -I gui -I gui/vendor/imgui ^
        -lopengl32 -lgdi32 -luser32 -lcomdlg32 -lshell32 -ldwmapi -limm32 -lws2_32 -lwinhttp ^
        -mwindows -o ARDiscordBypass_gui.exe
    strip ARDiscordBypass_gui.exe 2>nul
)

echo.
echo =========================================================
echo  Compilacao concluida com sucesso!
echo  CLI: ARDiscordBypass.exe
if "%BUILD_GUI%"=="1" echo  GUI: ARDiscordBypass_gui.exe
echo =========================================================
pause
