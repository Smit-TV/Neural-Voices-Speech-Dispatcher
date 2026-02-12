#!/usr/bin/env bash
set -e

# ====== НАСТРОЙКИ ======
CXX=g++
CXXFLAGS="-std=c++17 -O2"

SPEECHSDK_ROOT="./libs"
TARGET_PLATFORM="linux-x64"

SRC="./sd_module/*.cpp"
OUT="neuralvoices-${TARGET_PLATFORM}"

# ====== INCLUDE ======
INCLUDES="
-I${SPEECHSDK_ROOT}/build/native/include/cxx_api
-I${SPEECHSDK_ROOT}/build/native/include/c_api
-Ipackages/nlohmann.json.3.11.2/build/native/include
-I/usr/include/speech-dispatcher/
"

# ====== LIBRARY PATH ======
LIBDIR="
-L${SPEECHSDK_ROOT}/runtimes/${TARGET_PLATFORM}/native
"

# ====== LIBS ======
LIBS="
-lMicrosoft.CognitiveServices.Speech.core
-lspeechd_module
-lpugixml
-licuuc
"

RPATH="-Wl,-rpath,${SPEECHSDK_ROOT}/runtimes/${TARGET_PLATFORM}/native"

# ====== СБОРКА ======
$CXX $CXXFLAGS \
    $SRC \
    $INCLUDES \
    $LIBDIR \
    $LIBS \
    $RPATH \
    -o $OUT

echo "✔ Собрано: $OUT"
