project(vma_proj)
include(GNUInstallDirs)
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

message("vma not found, fetching from github")

FetchContent_Declare (
    vma_fetch
    GIT_REPOSITORY  https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG         v3.0.1        
    GIT_PROGRESS    TRUE
    #SOURCE_DIR ${PROJECT_SOURCE_DIR}/src    
    INSTALL_DIR     install
    INSTALL_COMMAND ${MAKE_EXECUTABLE} DESTDIR=<INSTALL_DIR>
    FIND_PACKAGE_ARGS NAMES VulkanMemoryAllocator 
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_LIST_DIR}/install
)
FetchContent_MakeAvailable (vma_fetch)

find_package(Vulkan REQUIRED)

target_link_libraries(VulkanMemoryAllocator PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(VulkanMemoryAllocator PUBLIC ${Vulkan_INCLUDE_DIR})

target_compile_options(VulkanMemoryAllocator PUBLIC $<$<CXX_COMPILER_ID:Clang>:-Wno-nullability-extension>) #supress WMA warnings...

set(vma_include_dirs "")
list(APPEND vma_include_dirs ${vma_fetch_SOURCE_DIR}/include)
set(vma_include_dirs "${vma_include_dirs}" PARENT_SCOPE)