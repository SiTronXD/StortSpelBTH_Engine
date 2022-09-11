project(tracy_proj)
include(GNUInstallDirs)
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

option(TRACY_NO_FRAME_IMAGE "" ON)

FetchContent_Declare (
    tracy_fetch
    GIT_REPOSITORY  https://github.com/wolfpld/tracy.git
    GIT_TAG         07a56f1        
    GIT_PROGRESS    TRUE
    #SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src
    INSTALL_DIR     install
    INSTALL_COMMAND ${MAKE_EXECUTABLE} DESTDIR=<INSTALL_DIR>
    FIND_PACKAGE_ARGS NAMES TracyClient
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_LIST_DIR}/install
)
FetchContent_MakeAvailable (tracy_fetch)


list(APPEND tracy_include_dirs ${tracy_fetch_SOURCE_DIR}/public )
list(APPEND tracy_include_dirs ${tracy_fetch_SOURCE_DIR}/public/tracy )
set(tracy_include_dirs "${tracy_include_dirs}" PARENT_SCOPE)