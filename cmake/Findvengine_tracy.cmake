
cmake_minimum_required(VERSION 3.21)

find_library(
    VENGINE_TRACY_LIBRARY
    NAMES tracy TracyClient
    HINTS ${PROJECT_SOURCE_DIR}/install/
    PATH_SUFFIXES  lib  bin  build/Release  build/Debug
)
find_path(
    VENGINE_TRACY_INCLUDE_DIR
    NAMES 
        #tracy
            Tracy.hpp
            TracyC.h
            TracyD3D11.hpp
            TracyD3D12.hpp
            TracyLua.hpp
            TracyOpenCL.hpp
            TracyOpenGL.hpp
            TracyVulkan.hpp
        #common 
            tracy_lz4.hpp
            tracy_lz4hc.hpp
            TracyAlign.hpp
            TracyAlloc.hpp
            TracyApi.h
            TracyColor.hpp
            TracyForceInline.hpp
            TracyMutex.hpp
            TracyProtocol.hpp
            TracyQueue.hpp
            TracySocket.hpp
            TracyStackFrames.hpp
            TracySystem.hpp
            TracyUwp.hpp
            TracyYield.hpp
        #client 
            tracy_concurrentqueue.h
            tracy_rpmalloc.hpp
            tracy_SPSCQueue.h
            TracyArmCpuTable.hpp
            TracyCallstack.h
            TracyCallstack.hpp
            TracyDebug.hpp
            TracyDxt1.hpp
            TracyFastVector.hpp
            TracyLock.hpp
            TracyProfiler.hpp
            TracyRingBuffer.hpp
            TracyScoped.hpp
            TracyStringHelpers.hpp
            TracySysTime.hpp
            TracySysTrace.hpp
            TracyThread.hpp

    HINTS ${PROJECT_SOURCE_DIR}/install/include/
    PATH_SUFFIXES client common tracy
)
include(FindPackageHandleStandardArgs)


find_package_handle_standard_args(
    vengine_tracy
    DEFAULT_MSG
    VENGINE_TRACY_LIBRARY
    VENGINE_TRACY_INCLUDE_DIR
)
mark_as_advanced(VENGINE_TRACY_LIBRARY VENGINE_TRACY_INCLUDE_DIR)
if(NOT TARGET vengine_tracy::vengine_tracy)
    add_library(vengine_tracy::vengine_tracy UNKNOWN IMPORTED )
    set_target_properties(vengine_tracy::vengine_tracy  PROPERTIES
               IMPORTED_LOCATION "${VENGINE_TRACY_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES 
                 "${VENGINE_TRACY_INCLUDE_DIR}"
               IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)
endif()