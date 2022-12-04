cmake_minimum_required(VERSION 3.23)
project(venginee
        VERSION 0.0.1
        DESCRIPTION "A toy cross platform renderer made with Vulkan"
        LANGUAGES CXX C
        
)
include(cmake/defs.cmake)
include(cmake/helper_funcs.cmake)
include(cmake/vengine_helpers.cmake)
include(cmake/program_dependencies.cmake)
include(cmake/setup_conan.cmake)

# SET(CMAKE_LINKER clang)
# #ASAN_OPTIONS=halt_on_error=0:detect_leaks=1 ./../build/debug_unix_lldb/3D_VEngine_GameDemo
# add_compile_options(-g -fsanitize=address,leak -fno-omit-frame-pointer -fsanitize-recover=address )
# add_link_options(   -g -fsanitize=address,leak -fno-omit-frame-pointer -fsanitize-memory-track-origins=2)

# Windows fix for creating a lib...
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(VERBOSE "Windows environment detected, overriding default symbol visibility to `visible` instead of `hidden`")
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

# Use CCACHE if it exists
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    # Compile with CCache if available to speed up compilation
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # TODO: Do not override "CMAKE_*" variables, it could've been declare before or after this... hmm?
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
else()
    message("Install Ccache to get faster build times!")
endif()

# Tell cmake to look for modules in our cmake folder 
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Setting up all definitions used within the cmake-files...
setup_vengine_definitions()

# Make sure Python and Conan are installed before continuing...
find_program_dependencies()

# Preparing compile options flags and definitions
set(CLANG_COMPILE_TIME_OPTIONS_debug 
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-msse2 -glldb -Wall -Wextra -Wpedantic -fno-omit-frame-pointer -fPIC>
    $<$<CXX_COMPILER_ID:GNU>:-msse2 -g -Wall -Wextra -fno-omit-frame-pointer>
    $<$<CXX_COMPILER_ID:MSVC>:/DEBUG /W4 /Zi /MD>
)
set(CLANG_COMPILE_TIME_OPTIONS_debugWCallStackProfiling ${CLANG_COMPILE_TIME_OPTIONS_debug})
set(CLANG_COMPILE_TIME_OPTIONS_release 
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O3 -msse2 -fPIC>
    $<$<CXX_COMPILER_ID:MSVC>:/O2 /MD>
)

if(${VENGINE_USE_PROFILING})
    message("Building profiling ENABLED!")
    set(CLANG_COMPILE_TIME_DEFINITIONS_debug                    "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "DEBUG"  )
    set(CLANG_COMPILE_TIME_DEFINITIONS_debugWCallStackProfiling "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "DEBUG"  "TRACY_CALLSTACK=5" )
    set(CLANG_COMPILE_TIME_DEFINITIONS_library                  "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "NDEBUG" )
    set(CLANG_COMPILE_TIME_DEFINITIONS_release                  "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "NDEBUG" )
    set(CLANG_COMPILE_TIME_OPTIONS_library ${CLANG_COMPILE_TIME_OPTIONS_release} "-fno-omit-frame-pointer")
else()
    message("Building profiling DISABLED!")
    add_compile_definitions("VENGINE_NO_PROFILING") # This must be added globally if user does not want to use profiling...
    set(CLANG_COMPILE_TIME_DEFINITIONS_debug                    "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "DEBUG"     )
    set(CLANG_COMPILE_TIME_DEFINITIONS_debugWCallStackProfiling "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "DEBUG"  "TRACY_CALLSTACK=5" )
    set(CLANG_COMPILE_TIME_DEFINITIONS_library                  "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "NDEBUG"    )
    set(CLANG_COMPILE_TIME_DEFINITIONS_release                  "GLM_FORCE_RADIANS" "GLM_FORCE_QUAT_DATA_WXYZ" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "NDEBUG"    )
    set(CLANG_COMPILE_TIME_OPTIONS_library ${CLANG_COMPILE_TIME_OPTIONS_release})
endif()

# all the source files
FILE(GLOB_RECURSE VENGINE_SOURCES CONFIGURE_DEPENDS
    ${VENGINE_SOURCE_DIR}/*.cpp 
    ${VENGINE_SOURCE_DIR}/*.h 
    ${VENGINE_SOURCE_DIR}/*.hpp
)
# all the source files
FILE(GLOB_RECURSE VENGINE_HEADERS CONFIGURE_DEPENDS    
    ${VENGINE_SOURCE_DIR}/*.h 
    ${VENGINE_SOURCE_DIR}/*.hpp 
)

# FILTER AWAY some files
list(FILTER VENGINE_SOURCES EXCLUDE REGEX
    "${VENGINE_SOURCE_DIR}/shared_memory/.*\\.h$"
)
#  FILTER AWAY some files
list(FILTER VENGINE_HEADERS EXCLUDE REGEX
    "${VENGINE_SOURCE_DIR}/shared_memory/.*\\.h$"    
)

# FILTER AWAY some files
list(FILTER VENGINE_SOURCES EXCLUDE REGEX
    "${VENGINE_SOURCE_DIR}/shared_memory/.*\\.cpp$"
)
#  FILTER AWAY some files
list(FILTER VENGINE_HEADERS EXCLUDE REGEX
    "${VENGINE_SOURCE_DIR}/shared_memory/.*\\.cpp$"
)
    
# Disable Fetching dependencies with conan and cmake FetchContent...
set(VENGINE_USE_GIT_SUBMODULES TRUE)
if(NOT (VENGINE_USE_GIT_SUBMODULES))
    # All imgui sources and headers
    FILE(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/*.cpp        
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_vulkan.cpp        
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_sdl.cpp)

    FILE(GLOB IMGUI_HEADERS CONFIGURE_DEPENDS            
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_vulkan.h            
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends/imgui_impl_sdl.h
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/*.h
    )
    # Add EnTT headers 
    FILE(GLOB ENTT_HEADERS CONFIGURE_DEPENDS            
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/EnTT/entt.hpp
    )
else()
    # All imgui sources and headers
    FILE(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/*.cpp        
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/backends/imgui_impl_vulkan.cpp        
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/backends/imgui_impl_sdl.cpp)

    FILE(GLOB IMGUI_HEADERS CONFIGURE_DEPENDS            
        ${CMAKE_CURRENT_SOURCE_DIVENGINE_USE_GIT_SUBMODULES}/deps/imgui/src/backends/imgui_impl_vulkan.h            
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/backends/imgui_impl_sdl.h
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/*.h
    )
    # Add EnTT headers 
    FILE(GLOB ENTT_HEADERS CONFIGURE_DEPENDS            
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/EnTT/src/single_include/entt/entt.hpp
    )
    # Add stb headers 
    FILE(GLOB STB_HEADERS CONFIGURE_DEPENDS            
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb/src/stb_image.h
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb/src/stb.h 
    )
    # Add stb headers 
    FILE(GLOB STB_HEADERS CONFIGURE_DEPENDS            
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb/src/stb_image.h
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb/src/stb.h 
    )

endif()

# Add vengine target as shared library
add_library(vengine SHARED  )
add_library(vengine::vengine ALIAS vengine )

target_include_directories(vengine 
    PUBLIC        
#    ${VENGINE_SOURCE_DIR}            
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/EnTT/src/single_include) 

# Find system dependencies 
find_package(Vulkan REQUIRED)


if( NOT (VENGINE_USE_GIT_SUBMODULES))
    message("Using Cmake FetchContent and Conan for dependencies!") 

    # Setup Conan, fetch required dependencies
    setup_conan_dependencies()

    #External Dependencies - fetched from github
    add_subdirectory(deps/assimp)
    add_subdirectory(deps/vma)
    add_subdirectory(deps/tracy)

    if(${VENGINE_USE_PROFILING})
        set(vengine_tracy TracyClient) 
    endif()

    #External Dependencies - fetched through conan
    target_link_libraries(vengine PUBLIC stb::stb)
    target_link_libraries(vengine PUBLIC glm::glm)
    target_link_libraries(vengine PUBLIC SDL2::SDL2)

    # setup the include directories for all external dependencies 
    target_include_directories(vengine PUBLIC ${imgui_include_dirs})
    target_include_directories(vengine PUBLIC ${tracy_include_dirs})
    target_include_directories(vengine PUBLIC ${vma_include_dirs})
    target_include_directories(vengine PUBLIC ${assimp_include_dirs})    

    set(vengine_stb "stb::stb")

else() # Use Git Submodules
    message("Using Git Submodules for dependencies!") 

    option(TRACY_ENABLE "" ON)    
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp_522)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/tracy/src)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/src)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/glm/src)
    #Sdl should be installed through systems own package manager...

    target_include_directories(vengine PRIVATE ${VENGINE_SOURCE_DIR}/../)    
    target_include_directories(vengine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp_522/include)
    #target_include_directories(vengine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp_522/src/include)
    target_include_directories(vengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/tracy/src/public)
    target_include_directories(vengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/src/include)
    target_include_directories(vengine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm/src/glm)
    target_include_directories(vengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/EnTT/src/single_include/entt)
    target_include_directories(vengine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb/src)
    
    find_package(SDL2 REQUIRED)
    target_include_directories(vengine PRIVATE ${SDL2_INCLUDE_DIRS})
    
    find_package(SFML REQUIRED network audio)    

    find_package(Lua REQUIRED)
    target_include_directories(vengine PRIVATE ${LUA_INCLUDE_DIR})

    # ADD OPENAL 
    find_package(OpenAL REQUIRED)
    target_link_libraries(vengine PRIVATE ${OPENAL_LIBRARY})
    message("wat is: " ${OPENAL_INCLUDE_DIR})
    target_include_directories(vengine PRIVATE ${OPENAL_INCLUDE_DIR})
    
    # ADD BULLET LIB 
    find_package(Bullet)    
    if(NOT ${BULLET_FOUND} )
        message( "Could not find Bullet!")        
        message(FATAL_ERROR "Bullet (Double Precision) must be installed through the distribution packet manager (tested with pacman!)")        
    endif()

    target_compile_definitions(vengine PUBLIC "BT_USE_DOUBLE_PRECISION")
    target_include_directories(vengine PUBLIC ${BULLET_INCLUDE_DIRS})    #Bullet needs these public...
    target_link_libraries(vengine PRIVATE ${BULLET_LIBRARIES})
    
    #set(vengine_vma VulkanMemoryAllocator) 
    set(vengine_stb "") #part of sources, no need to use as lib...


endif()

# Add the vengine sources + imgui sources to the vengine target
target_sources(vengine 
    PRIVATE 
    
    ${VENGINE_SOURCES} 
    ${IMGUI_HEADERS}    
    ${IMGUI_SOURCES}    
    ${ENTT_HEADERS}
    
    )

# Precompile header include 
target_include_directories(vengine PUBLIC  ${VENGINE_SOURCE_DIR}/pch)

# Linker Options 
target_link_options(vengine
    PRIVATE 
        $<$<CXX_COMPILER_ID:MSVC>:/subsystem:windows>
)

# Setting up Windows specific things for target vengine
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Include Vulkan header files from Vulkan SDK
	target_include_directories(vengine PRIVATE $ENV{VULKAN_SDK}/Include)

	# Link directory for vulkan-1
	target_link_directories(vengine PRIVATE $ENV{VULKAN_SDK}/Bin;$ENV{VULKAN_SDK}/Lib;)

    # Command to copy dll-files to build folder will begin after build...
    add_custom_command(TARGET vengine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:vengine> $<TARGET_FILE_DIR:vengine>
        COMMENT "Copying dll-files to location of executable..."
        COMMAND_EXPAND_LISTS
        )
    target_link_options(vengine
        PRIVATE 
            $<$<CXX_COMPILER_ID:MSVC>:/subsystem:console /NODEFAULTLIB:library>
    )
        
    set_target_properties(vengine PROPERTIES  
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# measure compiletime
set_property(TARGET vengine PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time") 

set_target_properties(vengine PROPERTIES            
                CXX_STANDARD 20                         # OBS! To use features from 11 and onward. Only use .hpp as header files...
                CXX_EXTENSIONS OFF                      # is this good?...
                EXPORT_COMPILE_COMMANDS ON 
                #INTERPROCEDURAL_OPTIMIZATION TRUE       # Will do link time optimizations if available
                LINK_WHAT_YOU_USE TRUE                  # Warns if something unncessary is linked                             
                CMAKE_CXX_INCLUDE_WHAT_YOU_USE TRUE     # Checks if a source file is used, if not then its not included...
                ) 
                                
# Setup Different Compiler options and Definitions for different build types...
target_compile_options(vengine PRIVATE "$<$<CONFIG:Debug>:${CLANG_COMPILE_TIME_OPTIONS_debug}>")
target_compile_options(vengine PRIVATE "$<$<CONFIG:Release>:${CLANG_COMPILE_TIME_OPTIONS_release}>")
target_compile_options(vengine PRIVATE "$<$<CONFIG:Library>:${CLANG_COMPILE_TIME_OPTIONS_library}>")
target_compile_options(vengine PRIVATE "$<$<CONFIG:DebugWCallStackProfiling>:${CLANG_COMPILE_TIME_OPTIONS_debugWCallStackProfiling}>")
target_compile_definitions(vengine PRIVATE "$<$<CONFIG:Debug>:${CLANG_COMPILE_TIME_DEFINITIONS_debug}>" )
target_compile_definitions(vengine PRIVATE "$<$<CONFIG:Release>:${CLANG_COMPILE_TIME_DEFINITIONS_release}>" )
target_compile_definitions(vengine PRIVATE "$<$<CONFIG:Library>:${CLANG_COMPILE_TIME_DEFINITIONS_library}>" )
target_compile_definitions(vengine PRIVATE "$<$<CONFIG:DebugWCallStackProfiling>:${CLANG_COMPILE_TIME_DEFINITIONS_debugWCallStackProfiling}>" )

# This config is setup in vscode workspace, in order to get the correct Compile_commands.json... (see: "cmake.configureArgs": ["-DCMAKE_BUILD_TYPE=cmake_project"],)
target_compile_definitions(vengine PRIVATE "$<$<CONFIG:cmake_project>:${CLANG_COMPILE_TIME_DEFINITIONS_debug}>" ) 
# NOTE! STB_IMAGE_IMPLEMENTATION  <-- Cant be used here! for some reason!

if(${CMAKE_BUILD_TYPE} MATCHES "Debug" OR ${CMAKE_BUILD_TYPE} MATCHES "Release" )
target_compile_definitions(vengine PRIVATE "_CONSOLE" )
target_compile_definitions(vengine PRIVATE "NO_MOUSE_GRAB" )
endif()

# Replace non portable functions with portable functions...
if(${UNIX})
    #define strcpy_s strcpy
    target_compile_definitions(vengine PRIVATE "strcpy_s=strcpy" )
endif()

target_compile_definitions(vengine PRIVATE "NO_VALIDATION")

# Link all libraries to vengine target
target_link_libraries(
    vengine 
    PUBLIC
    ${vengine_tracy}
    ${SDL2_LIBRARIES}
    sfml-network
    sfml-audio
    PRIVATE
    ${Vulkan_LIBRARIES}
    assimp::assimp
    ${vengine_stb}
    ${vengine_vma}
    ${LUA_LIBRARIES}
)



# Sources for executable demo
FILE(GLOB VENGINE_DEMO_SOURCES CONFIGURE_DEPENDS            
    demos/basic_demo/src/*.cpp    
    demos/basic_demo/src/*.hpp
    demos/basic_demo/src/*.h
    demos/basic_demo/main.cpp
)

# FILTER AWAY some files
list(FILTER VENGINE_DEMO_SOURCES EXCLUDE REGEX
    ".*/LevelEditorTestScene\\.h$"    
)

# FILTER AWAY some files
list(FILTER VENGINE_DEMO_SOURCES EXCLUDE REGEX
    ".*/LevelEditorTestScene\\.cpp$"
)

# Build demo, for developers
add_executable(vengine_demo
    ${VENGINE_DEMO_SOURCES}
)
set_target_properties(vengine_demo PROPERTIES            
                CXX_STANDARD 20                         # OBS! To use features from 11 and onward. Only use .hpp as header files...
                CXX_EXTENSIONS OFF                      # is this good?...
                EXPORT_COMPILE_COMMANDS ON 
                #INTERPROCEDURAL_OPTIMIZATION TRUE       # Will do link time optimizations if available
                LINK_WHAT_YOU_USE TRUE                  # Warns if something unncessary is linked                             
                CMAKE_CXX_INCLUDE_WHAT_YOU_USE TRUE     # Checks if a source file is used, if not then its not included...
                ) 
target_link_libraries(vengine_demo PUBLIC vengine)
target_include_directories(vengine_demo PRIVATE ${VENGINE_SOURCE_DIR}/../)
target_include_directories(vengine_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp_522/src/include)
target_include_directories(vengine_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp_522/include)
target_include_directories(vengine_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/tracy/src/public)
target_include_directories(vengine_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/src/include)
target_include_directories(vengine_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm/src/glm)
target_include_directories(vengine_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/EnTT/src/single_include/entt)
target_include_directories(vengine_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb/src)

# Make a target for generation of symbolic links, so we know when creation of it is needed
add_custom_target(generate_symbolic_links      ALL DEPENDS ${VENGINE_ASSET_LINK_DST})
add_custom_target(generate_symbolic_links_demo ALL DEPENDS ${VENGINE_DEMO_ASSET_LINK_DST})

set(VENGINE_INSTALL_ASSETS_PATH "not_set" CACHE STRING "If 'not_set' then the sample assets wont be installed anywhere.")
if(NOT (${VENGINE_INSTALL_ASSETS_PATH} MATCHES "not_set"))
    install_asset_dir(${VENGINE_INSTALL_ASSETS_PATH})
endif()

set(VENGINE_SYMLINK_TO_ASSETS_PATH "not_set" CACHE STRING "If 'not_set' then the assets dir wont have a symlink to given folder.")
if(NOT (${VENGINE_SYMLINK_TO_ASSETS_PATH} MATCHES "not_set"))
    if( NOT (${VENGINE_INSTALL_ASSETS_PATH} MATCHES "not_set"))
    create_assets_symlink_for_client(${VENGINE_INSTALL_ASSETS_PATH} ${VENGINE_SYMLINK_TO_ASSETS_PATH})
    else()
        message("Could not create symlink, VENGINE_INSTALL_ASSETS_PATH must be set. VENGINE_INSTALL_ASSETS_PATH is ${VENGINE_INSTALL_ASSETS_PATH} ")
    endif()
endif()

set(VENGINE_INSTALL_VENGINE_HELPER_PATH "not_set" CACHE STRING "vengine_helpers.cmake will be copied to the given location, useful for compiling shaders")
if(NOT (${VENGINE_INSTALL_VENGINE_HELPER_PATH} MATCHES "not_set"))
    install_vengine_helpers(${VENGINE_INSTALL_VENGINE_HELPER_PATH})
endif()

# Building shaders, to SPR-V
build_shaders(vengine ${VENGINE_ASSETS_DIR} ${VENGINE_ASSETS_DIR})


# In order to create Symbolic Links, add custom command, then a custom target, 
# lastly after set_exectuable make sure to add_dependencies to the target!
#create_symlink_buildtime(${VENGINE_ASSET_LINK_SRC} ${VENGINE_ASSET_LINK_DST})
if(${WIN32})
    add_custom_command(TARGET vengine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${VENGINE_ASSET_LINK_SRC} $<TARGET_FILE_DIR:vengine>/assets         
        COMMENT "Creating symbolic link ${VENGINE_ASSET_LINK_SRC} => $<TARGET_FILE_DIR:vengine>"        
        COMMAND_EXPAND_LISTS
        )
    if(${VENGINE_DEV_MODE})
        add_custom_command(TARGET vengine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${VENGINE_ASSET_LINK_SRC} ${CMAKE_CURRENT_LIST_DIR}/build/debug_win32/assets         
            COMMENT "Creating symbolic link ${VENGINE_ASSET_LINK_SRC} => ${CMAKE_CURRENT_LIST_DIR}/build/debug_win32/assets"        
            COMMAND_EXPAND_LISTS
            ) 
    endif()
else()
    create_symlink_now(${VENGINE_ASSET_LINK_SRC} ${VENGINE_ASSET_LINK_DST})
    create_symlink_now(${VENGINE_DEMO_ASSET_LINK_SRC} ${VENGINE_DEMO_ASSET_LINK_DST})
endif()

# setting up the required asset folder structure for the user
copy_assets_structure()