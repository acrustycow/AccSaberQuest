cmake_minimum_required(VERSION 3.22)
project(AccSaberQuest VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTERN_DIR ${CMAKE_CURRENT_LIST_DIR}/extern)

# QPM externals
include(${EXTERN_DIR}/extern.cmake)

file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_library(AccSaberQuest SHARED ${SOURCE_FILES})

target_include_directories(AccSaberQuest PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${EXTERN_DIR}/includes
)

target_link_libraries(AccSaberQuest PRIVATE
    ${EXTERN_DIR}/libs/libbsml.so
    ${EXTERN_DIR}/libs/libcustom-types.so
    log
    android
)

target_compile_options(AccSaberQuest PRIVATE
    -fvisibility=hidden
    -O2
)

target_compile_definitions(AccSaberQuest PRIVATE
    MOD_ID="AccSaberQuest"
    VERSION="0.1.0"
)
