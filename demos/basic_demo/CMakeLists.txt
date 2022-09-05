cmake_minimum_required(VERSION 3.23)
project(3D_VEngine_GameDemo VERSION 0.0.1
                    DESCRIPTION "Demo using 3D_Vengine library"
                    LANGUAGES CXX)

# Compile with CCache if available to speed up compilation
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()

# Setup C++ settings
set(USE_COMPILE_COMMANDS ON)

add_executable(3D_VEngine_GameDemo main.cpp ) #${SOURCES} 
set_property(TARGET 3D_VEngine_GameDemo PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time") # measure compiletime
target_compile_features(3D_VEngine_GameDemo PUBLIC cxx_std_20)

set_target_properties(3D_VEngine_GameDemo PROPERTIES
                #CXX_STANDARD ${CPP_LANGUANGE_STANDARD}
                CXX_EXTENSIONS OFF                      
                EXPORT_COMPILE_COMMANDS ${USE_COMPILE_COMMANDS} 
                INTERPROCEDURAL_OPTIMIZATION TRUE       # Will do link time optimizations if available
                LINK_WHAT_YOU_USE TRUE                          
                ) 

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


# == Downloading a model to load... ==
if(EXISTS ${CMAKE_SOURCE_DIR}/assets AND NOT (EXISTS ${CMAKE_SOURCE_DIR}/assets/models/ghost.obj))
    file(DOWNLOAD https://raw.githubusercontent.com/lr222gw/vengine_resources/main/models/ghost.obj ${CMAKE_SOURCE_DIR}/assets/models/ghost.obj)    
    file(DOWNLOAD https://raw.githubusercontent.com/lr222gw/vengine_resources/main/models/ghost.mtl ${CMAKE_SOURCE_DIR}/assets/models/ghost.mtl)   
    file(DOWNLOAD https://raw.githubusercontent.com/lr222gw/vengine_resources/main/textures/ghost.png ${CMAKE_SOURCE_DIR}/assets/textures/ghost.png)  
    file(DOWNLOAD https://raw.githubusercontent.com/lr222gw/vengine_resources/main/configs/simple_demo/config.cfg ${CMAKE_SOURCE_DIR}/assets/config.cfg)  
endif()

