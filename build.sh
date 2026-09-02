#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BUILD_TYPE="${1:-Release}"

echo "========================================="
echo "  ARDiscordBypass v2.0 - Build Linux"
echo "========================================="
echo ""

if [ ! -f "$SCRIPT_DIR/CMakeLists.txt" ]; then
    echo "ERRO: CMakeLists.txt nao encontrado em $SCRIPT_DIR"
    exit 1
fi

echo "[1/3] Configurando CMake ($BUILD_TYPE)..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo ""
echo "[2/3] Compilando..."
cmake --build "$BUILD_DIR" --parallel $(nproc)

EXE_PATH="$BUILD_DIR/ARDiscordBypass"

if [ ! -f "$EXE_PATH" ]; then
    echo "ERRO: Executavel nao foi gerado!"
    exit 1
fi

echo ""
echo "[3/3] Build concluido!"
echo ""
echo "Executavel: $EXE_PATH"
echo ""
echo "Para executar:"
echo "  $EXE_PATH"
echo ""
echo "Para informar caminho manual do Discord:"
echo "  $EXE_PATH /caminho/para/Discord"
