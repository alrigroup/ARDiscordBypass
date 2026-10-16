#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BUILD_TYPE="Release"
BUILD_GUI="OFF"

for arg in "$@"; do
    case "$arg" in
        --gui)
            BUILD_GUI="ON"
            ;;
        Debug|Release)
            BUILD_TYPE="$arg"
            ;;
    esac
done

echo "========================================="
echo "  ARDiscordBypass v3.5.0 - Build Linux"
echo "  Modo GUI Opcional: $BUILD_GUI"
echo "========================================="
echo ""

if [ ! -f "$SCRIPT_DIR/CMakeLists.txt" ]; then
    echo "ERRO: CMakeLists.txt nao encontrado em $SCRIPT_DIR"
    exit 1
fi

echo "[1/3] Configurando CMake ($BUILD_TYPE, GUI=$BUILD_GUI)..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_GUI="$BUILD_GUI"

echo ""
echo "[2/3] Compilando..."
cmake --build "$BUILD_DIR" --parallel $(nproc)

EXE_CLI="$BUILD_DIR/ARDiscordBypass"
EXE_GUI="$BUILD_DIR/ARDiscordBypass_gui"

if [ ! -f "$EXE_CLI" ]; then
    echo "ERRO: Executavel CLI nao foi gerado!"
    exit 1
fi

strip "$EXE_CLI" 2>/dev/null || true
cp "$EXE_CLI" "$SCRIPT_DIR/../ARDiscordBypass_linux"

echo ""
echo "[3/3] Build concluido com sucesso!"
echo "  ✔ CLI: $EXE_CLI"

if [ "$BUILD_GUI" = "ON" ] && [ -f "$EXE_GUI" ]; then
    strip "$EXE_GUI" 2>/dev/null || true
    cp "$EXE_GUI" "$SCRIPT_DIR/../ARDiscordBypass_gui_linux"
    echo "  ✔ GUI: $EXE_GUI"
fi

echo ""
echo "Para executar a versão CLI:"
echo "  ./ARDiscordBypass_linux"
if [ "$BUILD_GUI" = "ON" ]; then
    echo "Para executar a versão GUI:"
    echo "  ./ARDiscordBypass_gui_linux"
fi
