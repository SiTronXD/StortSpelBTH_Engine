project(assimp_proj)
include(GNUInstallDirs)
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

message("assimp not found, fetching from github")
option(ASSIMP_INSTALL "" OFF)
option(ASSIMP_BUILD_TESTS "" OFF)
option(ASSIMP_WARNINGS_AS_ERRORS "" OFF)

FetchContent_Declare (
    assimp_fetch
    GIT_REPOSITORY  https://github.com/lr222gw/assimp.git
    GIT_TAG         master
    GIT_PROGRESS    TRUE
    INSTALL_DIR     install
    INSTALL_COMMAND ${MAKE_EXECUTABLE} DESTDIR=<INSTALL_DIR>
    FIND_PACKAGE_ARGS NAMES assimp 
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_LIST_DIR}/install
)
FetchContent_MakeAvailable (assimp_fetch)

 set(assimp_include_dirs "")
 list(APPEND assimp_include_dirs ${assimp_fetch_SOURCE_DIR}/deps/assimp/src/include)
 list(APPEND assimp_include_dirs ${assimp_fetch_SOURCE_DIR}/deps/assimp/build/include)
 set(assimp_include_dirs "${assimp_include_dirs}" PARENT_SCOPE)
