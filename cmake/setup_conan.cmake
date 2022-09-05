
macro(setup_conan_dependencies IN_TARGET) # Use macro, to make sure it's applied to the callee

    include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)


    conan_cmake_autodetect(CONAN_SETTINGS)

    # Fetched cmake-config files will land in binary dir, look for them there...
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

    # Installing vma 
    # find_package(vma QUIET)
    # if(NOT vma_FOUND)        
    #     conan_cmake_configure(
    #     REQUIRES vulkan-memory-allocator/3.0.1
    #     GENERATORS cmake_find_package_multi         
    #     )
    #     conan_cmake_install(
    #         PATH_OR_REFERENCE .
    #         BUILD all
    #         SETTINGS ${CONAN_SETTINGS}
    #     )
    # endif() 
    # find_package(vulkan-memory-allocator REQUIRED)

    # Installing glm 
    find_package(glm QUIET)
    if(NOT glm_FOUND)        
        conan_cmake_configure(
        REQUIRES glm/cci.20220420
        GENERATORS cmake_find_package_multi
        )
        conan_cmake_install(
            PATH_OR_REFERENCE .
            BUILD all
            SETTINGS ${CONAN_SETTINGS}
        )        
    endif()     
    find_package(glm REQUIRED)
    if(NOT TARGET glm::glm)
        add_library(glm::glm ALIAS glm)
    endif()
    target_link_libraries(${IN_TARGET} PUBLIC glm::glm)

    # Installing stb 
    find_package(stb QUIET)
    if(NOT stb_FOUND)        
        conan_cmake_configure(
        REQUIRES stb/cci.20210910
        GENERATORS cmake_find_package_multi
        )
        conan_cmake_install(
            PATH_OR_REFERENCE .
            BUILD all
            SETTINGS ${CONAN_SETTINGS}
        )
    endif()         
    find_package(stb REQUIRED CONFIG)    
    if(NOT TARGET stb::stb)
        add_library(stb::stb ALIAS stb)
    endif()
    target_link_libraries(${IN_TARGET} PUBLIC stb::stb)

    # find_package(tracy QUIET)
    # if(NOT tracy_FOUND)    
    #     option(TRACY_NO_FRAME_IMAGE "" ON)
    #     conan_cmake_configure(
    #     REQUIRES tracy/0.8.2.1        
    #     GENERATORS cmake_find_package_multi        
    #     )
    #     conan_cmake_install(
    #         PATH_OR_REFERENCE .
    #         BUILD all            
    #         ENV CXX=/usr/bin/clang++
    #         SETTINGS ${CONAN_SETTINGS}
    #     )
    # endif()    
    # find_package(tracy REQUIRED CONFIG)
    
    #get_target_property(tracy_src tracy::tracy INTERFACE_INCLUDE_DIRECTORIES)
    #message("wats dis: " ${tracy_src})
    

    # Installing SDL through conan makes use of the SDL module in cmake...
    if(${WIN32} )
        find_package(SDL2 QUIET)
        if(NOT SDL2_FOUND)    
        
            message("SDL is missing, fetchin it with Conan!")
            conan_cmake_configure(
            REQUIRES sdl/2.0.20
            GENERATORS cmake_find_package_multi
            )
            conan_cmake_install(
                PATH_OR_REFERENCE .
                BUILD missing
                SETTINGS ${CONAN_SETTINGS}
            )
        endif()                
    endif()    
    # On Linux, we requires SDL2 to be installed through a packagemanager
    if(${UNIX})
        find_package(SDL2 QUIET)
        if(NOT SDL2_FOUND)    
                
            message(FATAL_ERROR "SDL2 is missing, please install it with your package manager!")                                        
        endif()                
    endif()
    find_package(SDL2 REQUIRED CONFIG)
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 ALIAS SDL2)
    endif()

    target_link_libraries(${IN_TARGET} PUBLIC SDL2::SDL2)

endmacro(setup_conan_dependencies)
