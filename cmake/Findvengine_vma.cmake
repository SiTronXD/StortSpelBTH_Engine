
cmake_minimum_required(VERSION 3.21)

find_library(
    VMA_LIBRARY
    NAMES vma VulkanMemoryAllocator
    HINTS ${PROJECT_SOURCE_DIR}/install/
    PATH_SUFFIXES  lib  bin  build/Release  build/Debug
)
find_path(
    VMA_INCLUDE_DIR
    NAMES vk_mem_alloc.h
    HINTS ${PROJECT_SOURCE_DIR}/install/include/
)
include(FindPackageHandleStandardArgs)


find_package_handle_standard_args(
    vengine_vma
    DEFAULT_MSG
    VMA_LIBRARY
    VMA_INCLUDE_DIR
)
mark_as_advanced(VMA_LIBRARY VMA_INCLUDE_DIR)
if(NOT TARGET vengine_vma::vengine_vma)
    add_library(vengine_vma::vengine_vma UNKNOWN IMPORTED )
    set_target_properties(vengine_vma::vengine_vma  PROPERTIES
               IMPORTED_LOCATION "${VMA_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES 
                 "${VMA_INCLUDE_DIR}"
               IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)
endif()