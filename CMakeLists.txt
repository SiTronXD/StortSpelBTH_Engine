cmake_minimum_required(VERSION 3.23)
project(3D_VEngine
        VERSION 0.0.1
        DESCRIPTION "A toy cross platform renderer made with Vulkan"
        LANGUAGES CXX
)
include(cmake/defs.cmake)
include(cmake/helper_funcs.cmake)
include(cmake/vengine_helpers.cmake)
include(cmake/program_dependencies.cmake)
include(cmake/setup_conan.cmake)

# Use CCACHE if it exists
# find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    # Compile with CCache if available to speed up compilation
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # TODO: Do not override "CMAKE_*" variables, it could've been declare before or after this... hmm?
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
else()
    message("Install Ccache to get faster build times!")
endif()

# Setting up all definitions used within the cmake-files...
setup_vengine_definitions()

# Make sure Python and Conan are installed before continuing...
find_program_dependencies()

message("1")
# User exposed Options
option(VENGINE_TRACY_BUILD_PROFILER "Builds the profiler tool (Windows = experimental)" ON) #ON/OF
option(VENGINE_USE_PROFILING "Build with tracy Profiling tools, enables capability to profile" ON) #ON/OF
message("2")
# Dropdown picker for Cmake GUI (Pick between build lib/exe)
SET(VENGINE_PICK_BUILD_EXE_OR_LIB "notAnOption" CACHE STRING "Select to build Library or Executable")
set_property(CACHE VENGINE_PICK_BUILD_EXE_OR_LIB PROPERTY STRINGS ${BUILD_AS_LIBRARY} ${BUILD_AS_EXECUTABLE} notAnOption)
if(${VENGINE_PICK_BUILD_EXE_OR_LIB} MATCHES "notAnOption")
    set(VENGINE_PICK_BUILD_EXE_OR_LIB ${DEFAULT_BUILD_EXE_OR_LIB} )
endif()
message("3")
# Setup C++ settings
# generate compile commands
message("4")
option(VENGINE_USE_COMPILE_COMMANDS "Generate compile_commands.json, used by clangd for c++..." ON)
if(NOT ${VENGINE_USE_COMPILE_COMMANDS})
message("CMAKE-WARNING: VENGINE_USE_COMPILE_COMMANDS is turned off! clangd will not work...")
endif()
message("5")





# Find Dependencies - CONAN 
#find_package(SDL2 2.0 REQUIRED)
find_package(Vulkan REQUIRED)
#find_package(tracy REQUIRED)

# find_package(Vulkan REQUIRED)
# find_package(SDL2 REQUIRED)



# include(cmake/defs.cmake)
# include(cmake/helper_funcs.cmake)
# include(cmake/setup_dependencies.cmake)
# include(cmake/vengine_helpers.cmake)
# include(FetchContent)

# libraries that should be installed on the OS...glfw and vulkan
#set(LIBRARIES "${SDL2_LIBRARIES};Vulkan::Vulkan")

# Compilation Settings 


# Initial Definitions of variables 
#setup_vengine_definitions()

# CMakeLists only Options 
#force_option(BUILD_DEFAULT_AS_LIBRARY  "If PICK_BUILD_EXE_OR_LIB is 'notAnOption', should Library be built by default?"  ON ) # Only changable in CMakeLists.txt ...
#mark_as_advanced(BUILD_DEFAULT_AS_LIBRARY)
#if(${BUILD_DEFAULT_AS_LIBRARY})
    #set(DEFAULT_BUILD_EXE_OR_LIB ${BUILD_AS_LIBRARY})
#else()
    #set(DEFAULT_BUILD_EXE_OR_LIB ${BUILD_AS_EXECUTABLE})
#endif()

# Preparing compile options flags and definitions
set(CLANG_COMPILE_TIME_OPTIONS_debug 
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-msse2 -glldb -Wall -Wextra -Wpedantic -fno-omit-frame-pointer>
    $<$<CXX_COMPILER_ID:GNU>:-msse2 -g -Wall -Wextra -fno-omit-frame-pointer>
    $<$<CXX_COMPILER_ID:MSVC>:/DEBUG /W4 /Zi>
)
set(CLANG_COMPILE_TIME_OPTIONS_debugWCallStackProfiling ${CLANG_COMPILE_TIME_OPTIONS_debug})
set(CLANG_COMPILE_TIME_OPTIONS_release 
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O3 -msse2 >
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)

message("6")
if(${VENGINE_USE_PROFILING})

    set(CLANG_COMPILE_TIME_DEFINITIONS_debug                    "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "DEBUG"  )
    set(CLANG_COMPILE_TIME_DEFINITIONS_debugWCallStackProfiling "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "DEBUG"  "TRACY_CALLSTACK=5" )
    set(CLANG_COMPILE_TIME_DEFINITIONS_library                  "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "NDEBUG" )
    set(CLANG_COMPILE_TIME_DEFINITIONS_release                  "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"   "TRACY_IMPORTS"  "TRACY_ENABLE=1" "NDEBUG" )
    set(CLANG_COMPILE_TIME_OPTIONS_library ${CLANG_COMPILE_TIME_OPTIONS_release} "-fno-omit-frame-pointer")
else()

    add_compile_definitions("VENGINE_NO_PROFILING") # This must be added globally if user does not want to use profiling...
    set(CLANG_COMPILE_TIME_DEFINITIONS_debug                    "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "DEBUG"     )
    set(CLANG_COMPILE_TIME_DEFINITIONS_debugWCallStackProfiling "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "DEBUG"  "TRACY_CALLSTACK=5" )
    set(CLANG_COMPILE_TIME_DEFINITIONS_library                  "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "NDEBUG"    )
    set(CLANG_COMPILE_TIME_DEFINITIONS_release                  "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE"     "VENGINE_NO_PROFILING"    "NDEBUG"    )
    set(CLANG_COMPILE_TIME_OPTIONS_library ${CLANG_COMPILE_TIME_OPTIONS_release})
endif()
message("7")
# Include Directories
#include_directories("${VENGINE_INCLUDES_DIR};${SDL2_INCLUDE_DIRS}")


# Setup folderstructure for VENGINE as library 
#if (${VENGINE_PICK_BUILD_EXE_OR_LIB} MATCHES ${BUILD_AS_LIBRARY}) 
    # Create Directories for library        
 #   copy_assets_structure()
#endif()

# all the source files
FILE(GLOB_RECURSE VENGINE_SOURCES CONFIGURE_DEPENDS
    ${VENGINE_SOURCE_DIR}/*.cpp 
    ${VENGINE_SOURCE_DIR}/*.h 
)
message("8")
add_executable(vengine)

# Setup Conan, fetch required dependencies
setup_conan_dependencies(vengine)

#External Dependencies 
Set(PREV_FETCHCONTENT_QUIET ${FETCHCONTENT_QUIET})

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


 # Add vengine target as shared library
add_library(vengine SHARED  )
add_library(vengine::vengine ALIAS vengine )

target_include_directories(vengine 
    PUBLIC 
    ${VENGINE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends
) 


message("before")
find_package(assimp QUIET)
message("assimp_found was: " ${assimp_FOUND})
if(NOT ${assimp_FOUND}) 
    message("Did not find assimp...")
    add_subdirectory(deps/assimp)
endif()
message("after")


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)


vengine_find(vengine_vma deps/vengine_vma VulkanMemoryAllocator)
# message("before")
# find_package(vengine_vma MODULE QUIET)
# message("vengine_vma_found was: " ${vengine_vma_FOUND})
# if(NOT ${vengine_vma_FOUND}) 
#     message("Did not find vengine_vma...")
#     add_subdirectory(deps/vma)
    
#     set(vengine_vma VulkanMemoryAllocator)
# else()
#     set(vengine_vma vengine_vma::vengine_vma)
# endif()
    
# Add the vengine sources + imgui sources to the vengine target
target_sources(vengine PRIVATE ${VENGINE_SOURCES} 
    ${IMGUI_HEADERS}    
    ${IMGUI_SOURCES}
    )

#include(deps/vmamemdump_dependency.cmake)
#add_subdirectory(deps/vengine_tracy)
vengine_find(vengine_tracy deps/vengine_tracy TracyClient)
#include(deps/tracyprofiler_dependency.cmake)
Set(FETCHCONTENT_QUIET ${PREV_FETCHCONTENT_QUIET})

if()
endif()

target_sources(vengine PRIVATE main.cpp ${VENGINE_SOURCES})
target_include_directories(vengine PUBLIC ${imgui_include_dirs})
#target_include_directories(vengine PUBLIC ${assimp_include_dirs})
target_include_directories(vengine PUBLIC ${tracy_include_dirs})
#target_link_directories(vengine PUBLIC tracy::tracy)
#target_link_directories(vengine PUBLIC ${vma_include_dirs})
#target_compile_definitions(vengine PUBLIC STB_IMAGE_IMPLEMENTATION STB_IMAGE_RESIZE_IMPLEMENTATION STBI_SSE2)
#target_link_directories(vengine PUBLIC vulkan-memory-allocator::vulkan-memory-allocator)
message("9")
# Linker Options 
target_link_options(vengine
    PRIVATE 
        $<$<CXX_COMPILER_ID:MSVC>:/subsystem:windows>
)

# Setting up Windows specific things
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

message("10")
# if( ${VENGINE_PICK_BUILD_EXE_OR_LIB} MATCHES "executable" )

#     add_executable(vengine main.cpp ${VENGINE_SOURCES} )

# elseif(${VENGINE_PICK_BUILD_EXE_OR_LIB} MATCHES "library") # library

#     add_subdirectory(${VENGINE_SOURCE_DIR})

#     # version of renderer.
#     set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#     # shared version
#     set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#     set_target_properties(vengine PROPERTIES         # TODO: Check if I use this... Dont think I do...
#         LIBRARY_OUTPUT_DIRECTORY ${VENGINE_AS_LIB_LIB}
#     )

#     # Copy all headers from src  folder to VENGINE_AS_LIBs Include folder...
#     file(GLOB IncludeFiles CONFIGURE_DEPENDS
#         ${VENGINE_SOURCE_DIR}/*.h            
#     )    
#     set_target_properties(vengine PROPERTIES PUBLIC_HEADER "${IncludeFiles}")
#     foreach(includeFile IN LISTS IncludeFiles)    
#         copy_files(${includeFile} ${VENGINE_AS_LIB_INCLUDE}/vengine )
#     endforeach()
# endif()

# measure compiletime
set_property(TARGET vengine PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time") 

# Setup Dependencies
# setup_tracy()   #Include Tracy
# setup_stb_lib() #Include STB image
# setup_glm_lib() #Include glm
# setup_imgui()   #Include imgui 
# setup_vma()     #Include VMA
# setup_assimp()  #include assimp

target_compile_features(vengine PUBLIC cxx_std_20)
set_target_properties(vengine PROPERTIES                
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

message("13")
target_link_libraries(
    vengine 
    PUBLIC 
    ${vengine_tracy}
    #TracyClient
    #tracy::tracy
    ${SDL2_LIBRARIES}
    PRIVATE
    #${LIBRARIES} 
    ${Vulkan_LIBRARIES}
    #${Vulkan_LIBRARY}
    #assimp::assimp
    assimp::assimp    
    stb::stb
    #vulkan-memory-allocator::vulkan-memory-allocator
    #VulkanMemoryAllocator.dll
    #VulkanMemoryAllocator    
    ${vengine_vma}
    #stb_lib
    #${LIBS_DIR}/libassimp.so
    #libpthread.a    # Tracy needs
    #libdl.a         # Tracy needs
    #VulkanMemoryAllocator
)
message("14")




# Make a target for generation of symbolic links, so we know when creation of it is needed
add_custom_target(generate_symbolic_links ALL DEPENDS ${VENGINE_ASSET_LINK_DST})

set(VENGINE_INSTALL_ASSETS_PATH "not_set" CACHE STRING "If 'not_set' then the sample assets wont be installed anywhere.")
if(NOT (${VENGINE_INSTALL_ASSETS_PATH} MATCHES "not_set"))
    install_asset_dir(${VENGINE_INSTALL_ASSETS_PATH})
endif()
message("15")
set(VENGINE_SYMLINK_TO_ASSETS_PATH "not_set" CACHE STRING "If 'not_set' then the assets dir wont have a symlink to given folder.")
if(NOT (${VENGINE_SYMLINK_TO_ASSETS_PATH} MATCHES "not_set"))
    if( NOT (${VENGINE_INSTALL_ASSETS_PATH} MATCHES "not_set"))
    create_assets_symlink_for_client(${VENGINE_INSTALL_ASSETS_PATH} ${VENGINE_SYMLINK_TO_ASSETS_PATH})
    else()
        message("Could not create symlink, VENGINE_INSTALL_ASSETS_PATH must be set. VENGINE_INSTALL_ASSETS_PATH is ${VENGINE_INSTALL_ASSETS_PATH} ")
    endif()
endif()
message("16")
set(VENGINE_INSTALL_VENGINE_HELPER_PATH "not_set" CACHE STRING "vengine_helpers.cmake will be copied to the given location, useful for compiling shaders")
if(NOT (${VENGINE_INSTALL_VENGINE_HELPER_PATH} MATCHES "not_set"))
    install_vengine_helpers(${VENGINE_INSTALL_VENGINE_HELPER_PATH})
endif()
message("17")
# Building shaders, to SPR-V
build_shaders(vengine ${VENGINE_ASSETS_DIR} ${VENGINE_ASSETS_DIR})
message("18")
# In order to create Symbolic Links, add custom command, then a custom target, 
# lastly after set_exectuable make sure to add_dependencies to the target!
#create_symlink_buildtime(${VENGINE_ASSET_LINK_SRC} ${VENGINE_ASSET_LINK_DST})
create_symlink_now(${VENGINE_ASSET_LINK_SRC} ${VENGINE_ASSET_LINK_DST})
message("19")




