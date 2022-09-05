project(tracy_proj)
include(GNUInstallDirs)

file(GLOB TRACY_INITIATED "${CMAKE_CURRENT_LIST_DIR}/deps/tracy/src/*")
list(LENGTH TRACY_INITIATED TRACY_DIR_EMPTY) # TRACY_DIR_EMPTY == 0 if empty...


option(TRACY_NO_FRAME_IMAGE "" ON)

#if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/tracy" OR "${TRACY_DIR_EMPTY}" EQUAL 0 )    #Add 'OR forceupdate' ... 
if(NOT EXISTS ${CMAKE_BINARY_DIR}/built_tracy)
    file(WRITE ${CMAKE_BINARY_DIR}/built_tracy "exists")
    include(FetchContent)
    message("tracy not found, fetching from github")
        
    #file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/deps/tracy)
    #file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/deps/tracy/build)
    FetchContent_Declare (
        tracy_fetch
        GIT_REPOSITORY  https://github.com/wolfpld/tracy.git
        GIT_TAG         07a56f1        
        GIT_PROGRESS    TRUE
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/tracy/src
        #BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/tracy/build
    )
    FetchContent_MakeAvailable (tracy_fetch)
    #FetchContent_Populate(tracy)
else()

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/tracy/src)
endif()    


set(tracy_include_dirs "")
list(APPEND tracy_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/tracy/src/public)
#list(APPEND tracy_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/tracy/build/include)




#add_library(tracy STATIC)
#
#set_target_properties(
#    tracy_lib
#    PROPERTIES VERSION ${PROJECT_VERSION}
#            SOVERSION ${PROJECT_VERSION_MAJOR}
#)
#target_compile_features(tracy_lib PUBLIC cxx_std_20)#? Needed or not? Does it destroy?
#
#
#
#target_sources(tracy_lib PRIVATE "${TRACY_SOURCES}" "${TRACY_HEADERS}")#? with or without quotes?
##target_include_directories(tracy_lib PUBLIC ${TRACY_HEADERS} ) #? with or without quotes?
#
#set(tracy_include_dirs "")
#list(APPEND tracy_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/tracy/include)
#target_include_directories(tracy_lib PUBLIC 
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/deps/tracy/include>    
#)
#
#
#set_property(
#    TARGET tracy_lib
#    PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE
#)

#Install the Lib
#install(TARGETS TracyClient
#    EXPORT TracyClient_export
#    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR} #Path defined in GNUInstallDirs
#)

##Install the Header files
#install(
#    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/deps/tracy
#    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} #Path defined in GNUInstallDirs
#)
#
##Generate import code; i.e. tracy_lib-config.cmake 
#install(
#    EXPORT tracy_export
#    FILE tracy-config.cmake
#    NAMESPACE tracy::
#    DESTINATION ${CMAKE_CURRENT_LIST_DIR}/cmake
#)
#
##Generate the tracy_lib-config-version.cmake file, with CMakePackageConfigHelpers
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#    "tracy-config-version.cmake"
#    COMPATIBILITY SameMajorVersion      # Use the same version as the project...
#)
#
#install(FILES 
#    "${CMAKE_CURRENT_BINARY_DIR}/tracy-config-version.cmake"
#    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/cmake"
#)



