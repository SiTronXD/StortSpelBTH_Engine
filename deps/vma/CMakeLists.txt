project(vma_proj)
include(GNUInstallDirs)

file(GLOB VMA_INITIATED "${CMAKE_CURRENT_LIST_DIR}/deps/vma/src/*")
list(LENGTH VMA_INITIATED VMA_DIR_EMPTY) # VMA_DIR_EMPTY == 0 if empty...

#option(VMA_STATIC_VULKAN_FUNCTIONS "" OFF)
#option(VMA_DYNAMIC_VULKAN_FUNCTIONS "" ON)

#if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/vma" OR "${VMA_DIR_EMPTY}" EQUAL 0 )    #Add 'OR forceupdate' ... 
# if(NOT EXISTS ${CMAKE_BINARY_DIR}/built_vma)
#     file(WRITE ${CMAKE_BINARY_DIR}/built_vma "exists")
#     include(FetchContent)
#     message("vma not found, fetching from github")
        
#     #file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/vma)
#     #file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/vma/build)
#     FetchContent_Declare (
#         vma_fetch
#         GIT_REPOSITORY  https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
#         GIT_TAG         v3.0.1        
#         GIT_PROGRESS    TRUE
#         SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/vma/src
#         #BINARY_DIR ${PROJECT_SOURCE_DIR}/deps/vma/build
#     )
#     FetchContent_MakeAvailable (vma_fetch)
#     #FetchContent_Populate(vma)
# else()

#     add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/vma/src)
# endif()    



include(FetchContent)
message("vma not found, fetching from github")
    
#file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/vma)
#file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/vma/build)
FetchContent_Declare (
    vma_fetch
    GIT_REPOSITORY  https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG         v3.0.1        
    GIT_PROGRESS    TRUE
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/src
    #BINARY_DIR ${PROJECT_SOURCE_DIR}/deps/vma/build
    INSTALL_DIR     install
    INSTALL_COMMAND ${MAKE_EXECUTABLE} DESTDIR=<INSTALL_DIR>
    FIND_PACKAGE_ARGS NAMES VulkanMemoryAllocator 
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_LIST_DIR}/install
)
FetchContent_MakeAvailable (vma_fetch)




find_package(Vulkan REQUIRED)

#target_link_libraries(VulkanMemoryAllocator PRIVATE ${Vulkan_LIBRARY})
target_link_libraries(VulkanMemoryAllocator PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(VulkanMemoryAllocator PUBLIC ${Vulkan_INCLUDE_DIR})

target_compile_options(VulkanMemoryAllocator PUBLIC $<$<CXX_COMPILER_ID:Clang>:-Wno-nullability-extension>) #supress WMA warnings...

set(vma_include_dirs "")
list(APPEND vma_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/vma/src/include)
#list(APPEND vma_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/vma/build/include)

    

# add_library(vma_lib STATIC)

# set_target_properties(
#     vma_lib
#     PROPERTIES VERSION ${PROJECT_VERSION}
#             SOVERSION ${PROJECT_VERSION_MAJOR}
# )
# target_compile_features(vma_lib PUBLIC cxx_std_20)#? Needed or not? Does it destroy?



# target_sources(vma_lib PRIVATE "${VMA_SOURCES}" "${VMA_HEADERS}")#? with or without quotes?
# #target_include_directories(vma_lib PUBLIC ${VMA_HEADERS} ) #? with or without quotes?

# set(vma_include_dirs "")
# list(APPEND vma_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/vma/include)
# target_include_directories(vma_lib PUBLIC 
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/deps/vma/include>    
# )


# set_property(
#     TARGET vma_lib
#     PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE
# )

# #Install the Lib
# install(TARGETS vma_lib
#     EXPORT vma_lib_export
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR} #Path defined in GNUInstallDirs
# )

# #Install the Header files
# install(
#     DIRECTORY ${PROJECT_SOURCE_DIR}/deps/vma
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} #Path defined in GNUInstallDirs
# )

# #Generate import code; i.e. vma_lib-config.cmake 
# install(
#     EXPORT vma_lib_export
#     FILE vma_lib-config.cmake
#     NAMESPACE vma_lib::
#     DESTINATION ${PROJECT_SOURCE_DIR}/cmake
# )

# #Generate the vma_lib-config-version.cmake file, with CMakePackageConfigHelpers
# include(CMakePackageConfigHelpers)
# write_basic_package_version_file(
#     "vma_lib-config-version.cmake"
#     COMPATIBILITY SameMajorVersion      # Use the same version as the project...
# )

# install(FILES 
#     "${CMAKE_CURRENT_BINARY_DIR}/vma_lib-config-version.cmake"
#     DESTINATION "${PROJECT_SOURCE_DIR}/cmake"
# )



