cmake_minimum_required(VERSION 3.23)
project(3D_VEngine_GameDemo VERSION 0.0.1 DESCRIPTION "Demo using 3D_Vengine library" LANGUAGES CXX)

add_executable(3D_VEngine_GameDemo main.cpp ) 
target_compile_features(3D_VEngine_GameDemo PUBLIC cxx_std_20)

# == FETCHING VENGINE ==
# Everything under this line is everyhing needed to fetch and use VEngine (except the downloading model part...)
include(FetchContent)
FetchContent_Declare(
    vengine
    GIT_REPOSITORY  git@github.com:lr222gw/3D_VEngine.git
    GIT_TAG         master                                
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
)
set(VENGINE_INSTALL_ASSETS_PATH          ${CMAKE_SOURCE_DIR}        CACHE INTERNAL "Installation location of the assets folder")
set(VENGINE_SYMLINK_TO_ASSETS_PATH       ${CMAKE_BINARY_DIR}        CACHE INTERNAL "Creates a symlink from the assets path to the build dir, so no copying of assets is needed")
set(VENGINE_INSTALL_VENGINE_HELPER_PATH  ${CMAKE_SOURCE_DIR}/cmake  CACHE INTERNAL "Installation of vengine_helpers.cmake which contains cmake functions to build shaders.")
FetchContent_MakeAvailable (vengine)

include(cmake/vengine_helpers.cmake)    # This file will be installed to VENGINE_INSTALL_VENGINE_HELPER_PATH. It will first be available after cmake has done a configure...
build_shaders(3D_VEngine_GameDemo ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/assets) # Build shaders, source files and compiled files will be in the same folder
    
target_link_libraries(3D_VEngine_GameDemo PUBLIC 3D_VEngine::3D_VEngine)


