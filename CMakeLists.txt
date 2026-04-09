cmake_minimum_required(VERSION 3.16)
project(neuralvoices)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SPEECHSDK_ROOT "${CMAKE_SOURCE_DIR}/libs")
set(TARGET_PLATFORM "linux-x64")

file(GLOB SRC_FILES
    "${CMAKE_SOURCE_DIR}/sd_module/*.*pp"
)

add_executable(neuralvoices-${TARGET_PLATFORM} ${SRC_FILES})

# ====== ПОИСК SPEECH-DISPATCHER ======
find_path(SPEECHD_INCLUDE_DIR
    NAMES spd_module_main.h
    PATHS
        /usr/include
        /usr/include/speech-dispatcher
)

if (NOT SPEECHD_INCLUDE_DIR)
    message(FATAL_ERROR "speech-dispatcher headers not found (spd_module_main.h)")
endif()

# ====== INCLUDE ======
target_include_directories(neuralvoices-${TARGET_PLATFORM} PRIVATE
    ${SPEECHSDK_ROOT}/build/native/include/cxx_api
    ${SPEECHSDK_ROOT}/build/native/include/c_api
    ${CMAKE_SOURCE_DIR}/packages/nlohmann.json.3.11.2/build/native/include
    ${SPEECHD_INCLUDE_DIR}
)

# ====== LIBRARY PATH ======
target_link_directories(neuralvoices-${TARGET_PLATFORM} PRIVATE
    ${SPEECHSDK_ROOT}/runtimes/${TARGET_PLATFORM}/native
)

# ====== ПОИСК БИБЛИОТЕК ======
find_library(SPEECHSDK_LIB
    NAMES Microsoft.CognitiveServices.Speech.core
    PATHS ${SPEECHSDK_ROOT}/runtimes/${TARGET_PLATFORM}/native
)

find_library(SPEECHD_MODULE_LIB NAMES speechd_module)
find_library(PUGIXML_LIB NAMES pugixml)
find_library(ICUUC_LIB NAMES icuuc)
if (NOT SPEECHSDK_LIB)
    message(FATAL_ERROR "Speech SDK lib not found")
endif()

# ====== ЛИНКОВКА ======
target_link_libraries(neuralvoices-${TARGET_PLATFORM} PRIVATE
    ${SPEECHSDK_LIB}
    ${SPEECHD_MODULE_LIB}
    ${PUGIXML_LIB}
    ${ICUUC_LIB}
)

# ====== RPATH ======
set_target_properties(neuralvoices-${TARGET_PLATFORM} PROPERTIES
    BUILD_RPATH "${SPEECHSDK_ROOT}/runtimes/${TARGET_PLATFORM}/native"
)