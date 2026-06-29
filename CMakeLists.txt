cmake_minimum_required(VERSION 3.22)
project(AccSaberQuest VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTERN_DIR ${CMAKE_CURRENT_LIST_DIR}/extern)
set(QPM_CACHE $ENV{HOME}/.local/share/QPM-RS/cache)

set(SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/AccSaberAPI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Hooks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/MenuPanelController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ResultsViewController.cpp
)

add_library(AccSaberQuest SHARED ${SOURCE_FILES})

target_include_directories(AccSaberQuest PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${EXTERN_DIR}/includes
    ${QPM_CACHE}/beatsaber-hook/6.4.2/tmp/shared
    ${QPM_CACHE}/beatsaber-hook/6.4.2/tmp/shared/rapidjson/include
    ${QPM_CACHE}/bs-cordl/4008.0.0/tmp/include
    ${QPM_CACHE}/bsml/0.4.55/tmp/include
    ${QPM_CACHE}/custom-types/0.18.4/tmp/include
    ${QPM_CACHE}/paper2_scotland2/4.7.0/tmp/shared
    ${QPM_CACHE}/scotland2/0.1.7/tmp/shared
    ${QPM_CACHE}/fmt/11.0.2/tmp/fmt/include
    ${QPM_CACHE}/tinyxml2/10.0.0/tmp/include
)

target_link_libraries(AccSaberQuest PRIVATE
    ${EXTERN_DIR}/libs/libbeatsaber-hook.so
    ${EXTERN_DIR}/libs/libbsml.so
    ${EXTERN_DIR}/libs/libcustom-types.so
    ${EXTERN_DIR}/libs/libpaper2_scotland2.so
    ${EXTERN_DIR}/libs/libsl2.so
    log
    android
)

target_compile_definitions(AccSaberQuest PRIVATE
    MOD_ID="AccSaberQuest"
    VERSION="0.1.0"
)
