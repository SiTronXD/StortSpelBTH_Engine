include(GNUInstallDirs)

file(GLOB IMGUI_INITIATED "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/*")
list(LENGTH IMGUI_INITIATED IMGUI_DIR_EMPTY) # IMGUI_DIR_EMPTY == 0 if empty...

find_package(Vulkan REQUIRED)
#if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui" OR "${IMGUI_DIR_EMPTY}" EQUAL 0)    #Add 'OR forceupdate' ... 

if(NOT EXISTS ${CMAKE_BINARY_DIR}/built_imgui)
    file(WRITE ${CMAKE_BINARY_DIR}/built_imgui "exists")
    include(FetchContent)
    message("imgui not found, fetching from github")
    FetchContent_Declare (
        imgui_fetch
        GIT_REPOSITORY  https://github.com/ocornut/imgui.git
        GIT_TAG         8cbd391 # 8cbd391 = currently newest commit on Docking branch...
        # 9aae45e = release 1.88, use 'master' for latest...
        #GIT_SHALLOW     TRUE
        GIT_PROGRESS    TRUE
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/imgui/src
        #BINARY_DIR ${PROJECT_SOURCE_DIR}/deps/imgui
    )
    FetchContent_MakeAvailable (imgui_fetch)
    #FetchContent_Populate(imgui)

    FILE(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS
        ${imgui_fetch_SOURCE_DIR}/*.cpp        
        ${imgui_fetch_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp        
        ${imgui_fetch_SOURCE_DIR}/backends/imgui_impl_sdl.cpp)

    FILE(GLOB IMGUI_HEADERS CONFIGURE_DEPENDS            
        ${imgui_fetch_SOURCE_DIR}/backends/imgui_impl_vulkan.h            
        ${imgui_fetch_SOURCE_DIR}/backends/imgui_impl_sdl.h
        ${imgui_fetch_SOURCE_DIR}/*.h
    )

else()

    file(GLOB IMGUI_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/deps/imgui/src/*.h
    ${CMAKE_SOURCE_DIR}/deps/imgui/src/backends/imgui_impl_vulkan.h
    ${CMAKE_SOURCE_DIR}/deps/imgui/src/backends/imgui_impl_sdl.h    
    )
    file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/deps/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/deps/imgui/src/backends/imgui_impl_vulkan.cpp
    ${CMAKE_SOURCE_DIR}/deps/imgui/src/backends/imgui_impl_sdl.cpp
    )

endif()    
    

add_library(imgui_lib STATIC)

set_target_properties(
    imgui_lib
    PROPERTIES VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_features(imgui_lib PUBLIC cxx_std_20)#? Needed or not? Does it destroy?

target_sources(imgui_lib PRIVATE "${IMGUI_SOURCES}" "${IMGUI_HEADERS}")#? with or without quotes?
#target_include_directories(imgui_lib PUBLIC ${IMGUI_HEADERS} ) #? with or without quotes?
set(imgui_include_dirs "")
list(APPEND imgui_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src)
list(APPEND imgui_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/backends)
target_include_directories(imgui_lib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src> #? Add backends? (separated with ',' ...)
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/src/backends> 
)

#Imgui uses both Vulkan and SDL, so we need to link those libraries...
target_link_libraries(imgui_lib PRIVATE SDL2::SDL2)
#target_link_libraries(imgui_lib PRIVATE Vulkan::Vulkan)
target_link_libraries(imgui_lib PRIVATE ${Vulkan_LIBRARY})
#Imgui uses both SDL and Vulkan headers that does not seem to be included when linking libraries on windows...
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
    DIRECTORY ${PROJECT_SOURCE_DIR}/deps/imgui/src
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



