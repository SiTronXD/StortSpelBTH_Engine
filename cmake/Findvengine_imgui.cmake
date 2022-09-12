
cmake_minimum_required(VERSION 3.21)

find_library(
    VENGINE_IMGUI_LIBRARY
    NAMES imgui_lib imgui_libd
    HINTS ${PROJECT_SOURCE_DIR}/install/
    PATH_SUFFIXES  lib  bin  build/Release  build/Debug
)
find_path(
    VENGINE_IMGUI_INCLUDE_DIR
    NAMES 
        #root 
            imconfig.h
            imgui_internal.h
            imgui.h
            imstb_rectpack.h
            imstb_textedit.h
            imstb_truetype.h
        #backends 
            imgui_impl_sdl.h
            imgui_impl_vulkan.h
    HINTS ${PROJECT_SOURCE_DIR}/install/include/src     
)
include(FindPackageHandleStandardArgs)


find_package_handle_standard_args(
    vengine_imgui
    DEFAULT_MSG
    VENGINE_IMGUI_LIBRARY
    VENGINE_IMGUI_INCLUDE_DIR
)
mark_as_advanced(VENGINE_IMGUI_LIBRARY VENGINE_IMGUI_INCLUDE_DIR)
if(NOT TARGET vengine_imgui::vengine_imgui)
    add_library(vengine_imgui::vengine_imgui UNKNOWN IMPORTED )
    set_target_properties(vengine_imgui::vengine_imgui  PROPERTIES
               IMPORTED_LOCATION "${VENGINE_IMGUI_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES 
                 "${VENGINE_IMGUI_INCLUDE_DIR}"
               IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)
endif()