project(assimp_proj)

include(GNUInstallDirs)

file(GLOB ASSIMP_INITIATED "${CMAKE_CURRENT_LIST_DIR}/deps/assimp/src/*")
list(LENGTH ASSIMP_INITIATED ASSIMP_DIR_EMPTY) # ASSIMP_DIR_EMPTY == 0 if empty...


option(ASSIMP_INSTALL "" OFF)
option(ASSIMP_BUILD_TESTS "" OFF)
option(ASSIMP_WARNINGS_AS_ERRORS "" OFF)

#if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/assimp" OR "${ASSIMP_DIR_EMPTY}" EQUAL 0 )    #Add 'OR forceupdate' ... 
# if(NOT EXISTS ${CMAKE_BINARY_DIR}/built_assimp)
#     file(WRITE ${CMAKE_BINARY_DIR}/built_assimp "exists")
#     include(FetchContent)
#     message("assimp not found, fetching from github")
        
#     #file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/assimp)
#     #file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/assimp/build)
#     FetchContent_Declare (
#         assimp_fetch
#         GIT_REPOSITORY  https://github.com/lr222gw/assimp.git
#         GIT_TAG         master        
#         GIT_PROGRESS    TRUE
#         INSTALL_DIR     ${CMAKE_CURRENT_LIST_DIR}/install
#         #SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/assimp/src
#         #BINARY_DIR ${PROJECT_SOURCE_DIR}/deps/assimp/build
#     )
#     FetchContent_MakeAvailable (assimp_fetch)
#     #FetchContent_Populate(assimp)
# else()

#     add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/assimp/src)
# endif()    

include(FetchContent)
message("assimp not found, fetching from github")

find_package(assimp)
message("foundit? " ${assimp_FOUND})

if(NOT ${assimp_FOUND})
FetchContent_Declare (
    assimp_fetch
    GIT_REPOSITORY  https://github.com/lr222gw/assimp.git
    GIT_TAG         master
    GIT_PROGRESS    TRUE
    INSTALL_DIR     install
    INSTALL_COMMAND ${MAKE_EXECUTABLE} DESTDIR=<INSTALL_DIR>
    #INSTALL_COMMAND ${MAKE_EXECUTABLE} DESTDIR=<INSTALL_DIR>
        #^ TODO: Looks weird? 
    #SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/assimp/src
    #BINARY_DIR ${PROJECT_SOURCE_DIR}/deps/assimp/build
    FIND_PACKAGE_ARGS NAMES assimp 
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_LIST_DIR}/install
)
FetchContent_MakeAvailable (assimp_fetch)

set(assimp_include_dirs "")
list(APPEND assimp_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/assimp/src/include)
list(APPEND assimp_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/assimp/build/include)
endif()





# install(TARGETS assimp
#         DESTINATION ${CMAKE_CURRENT_LIST_DIR}/lib
#         #PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE    
        
#     )

# install(
#     DIRECTORY ${assimp_fetch_SOURCE_DIR}/include/ ${assimp_fetch_BINARY_DIR}/include/
#     DESTINATION ${CMAKE_CURRENT_LIST_DIR}/include
# )

# install(
#     EXPORT assimp_export
#     FILE assimp-config.cmake
#     NAMESPACE assimp_export::
#     DESTINATION ${PROJECT_SOURCE_DIR}/cmake
# )




#find_package(assimp REQUIRED)
#message("wat was: " ${assimp_FOUND})
# add_library(assimp_lib STATIC)

# set_target_properties(
#     assimp_lib
#     PROPERTIES VERSION ${PROJECT_VERSION}
#             SOVERSION ${PROJECT_VERSION_MAJOR}
# )
# target_compile_features(assimp_lib PUBLIC cxx_std_20)#? Needed or not? Does it destroy?



# target_sources(assimp_lib PRIVATE "${ASSIMP_SOURCES}" "${ASSIMP_HEADERS}")#? with or without quotes?
# #target_include_directories(assimp_lib PUBLIC ${ASSIMP_HEADERS} ) #? with or without quotes?

# set(assimp_include_dirs "")
# list(APPEND assimp_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/assimp/include)
# target_include_directories(assimp_lib PUBLIC 
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/deps/assimp/include>    
# )


# set_property(
#     TARGET assimp_lib
#     PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE
# )

# #Install the Lib
# install(TARGETS assimp_lib
#     EXPORT assimp_lib_export
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR} #Path defined in GNUInstallDirs
# )

# #Install the Header files
# install(
#     DIRECTORY ${PROJECT_SOURCE_DIR}/deps/assimp
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} #Path defined in GNUInstallDirs
# )

# #Generate import code; i.e. assimp_lib-config.cmake 
# install(
#     EXPORT assimp_lib_export
#     FILE assimp_lib-config.cmake
#     NAMESPACE assimp_lib::
#     DESTINATION ${PROJECT_SOURCE_DIR}/cmake
# )

# #Generate the assimp_lib-config-version.cmake file, with CMakePackageConfigHelpers
# include(CMakePackageConfigHelpers)
# write_basic_package_version_file(
#     "assimp_lib-config-version.cmake"
#     COMPATIBILITY SameMajorVersion      # Use the same version as the project...
# )

# install(FILES 
#     "${CMAKE_CURRENT_BINARY_DIR}/assimp_lib-config-version.cmake"
#     DESTINATION "${PROJECT_SOURCE_DIR}/cmake"
# )



