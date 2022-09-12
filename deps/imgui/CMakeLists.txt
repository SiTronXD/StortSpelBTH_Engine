cmake_minimum_required(VERSION 3.23) #Neseccary? Maybe remove...
project(imgui_proj 
	VERSION 0.0.1) # UGLY FIX! 
include(GNUInstallDirs)


find_package(Vulkan REQUIRED)



FILE(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp        
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp        
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_sdl.cpp)

FILE(GLOB IMGUI_HEADERS CONFIGURE_DEPENDS            
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_vulkan.h            
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_sdl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)


add_library(imgui_lib SHARED)

set_target_properties(
    imgui_lib
    PROPERTIES VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_features(imgui_lib PUBLIC cxx_std_20)

target_sources(imgui_lib PRIVATE "${IMGUI_SOURCES}" "${IMGUI_HEADERS}")

FILE(GLOB IMGUI_HEADERS CONFIGURE_DEPENDS            
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_vulkan.h            
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_sdl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)


set(imgui_include_dirs "")
list(APPEND imgui_include_dirs ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND imgui_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/backends)
set(imgui_include_dirs "${imgui_include_dirs}" PARENT_SCOPE)

target_include_directories(imgui_lib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends> 
)

target_link_libraries(imgui_lib PUBLIC SDL2::SDL2)
target_link_libraries(imgui_lib PUBLIC ${Vulkan_LIBRARY})
target_include_directories(imgui_lib PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(imgui_lib PUBLIC ${SDL2_INCLUDE_DIRS})

set_property(
    TARGET imgui_lib
    PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)

#Install the Lib
install(TARGETS imgui_lib
    EXPORT imgui_lib_export
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR} #Path defined in GNUInstallDirs
)

#Install the Header files
install(
    #DIRECTORY ${PROJECT_SOURCE_DIR}/src
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}    
    
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} #Path defined in GNUInstallDirs
)

#Generate import code; i.e. imgui_lib-config.cmake 
install(
    EXPORT imgui_lib_export
    FILE imgui_lib-config.cmake
    NAMESPACE imgui_lib::
    DESTINATION ${PROJECT_SOURCE_DIR}/cmake
)

#Generate the imgui_lib-config-version.cmake file, with CMakePackageConfigHelpers
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "imgui_lib-config-version.cmake"
    COMPATIBILITY SameMajorVersion      # Use the same version as the project...
)

install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/imgui_lib-config-version.cmake"
    DESTINATION "${PROJECT_SOURCE_DIR}/cmake"
)