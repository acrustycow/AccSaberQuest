cmake_minimum_required(VERSION 3.22)
project(AccSaberQuest VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── QPM externals ──────────────────────────────────────────────────────────
include(${CMAKE_CURRENT_LIST_DIR}/extern/extern.cmake OPTIONAL RESULT_VARIABLE extern_result)
if(NOT extern_result)
    message(FATAL_ERROR "extern.cmake not found – run 'qpm restore' first")
endif()

# ── Source files ───────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_library(AccSaberQuest SHARED ${SOURCE_FILES})

target_include_directories(AccSaberQuest PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${EXTERN_INC_DIR}
)

target_link_libraries(AccSaberQuest PRIVATE
    ${EXTERN_LIB_DIR}/libbsml.so
    ${EXTERN_LIB_DIR}/libcustom-types.so
    log
    android
)

# ── Android/ABI settings ───────────────────────────────────────────────────
target_compile_options(AccSaberQuest PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -O2
)

target_compile_definitions(AccSaberQuest PRIVATE
    MOD_ID="AccSaberQuest"
    VERSION="0.1.0"
)

# ── QMOD packaging ─────────────────────────────────────────────────────────
set(QMOD_OUTPUT_DIR ${CMAKE_BINARY_DIR}/qmod_staging)

add_custom_command(TARGET AccSaberQuest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${QMOD_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:AccSaberQuest> ${QMOD_OUTPUT_DIR}/libAccSaberQuest.so
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/mod.json ${QMOD_OUTPUT_DIR}/mod.json
    COMMAND ${CMAKE_COMMAND} -E chdir ${QMOD_OUTPUT_DIR}
            ${CMAKE_COMMAND} -E tar cf ${CMAKE_BINARY_DIR}/AccSaberQuest.qmod --format=zip .
    COMMENT "Packaging AccSaberQuest.qmod"
)
