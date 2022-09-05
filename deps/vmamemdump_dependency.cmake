include(GNUInstallDirs)

file(GLOB VMA_INITIATED "${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/*")
list(LENGTH VMA_INITIATED VMA_DIR_EMPTY) # VMA_DIR_EMPTY == 0 if empty...


if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/vma" OR NOT("${VMA_DIR_EMPTY}" EQUAL 0) )    #Add 'OR forceupdate' ... 

    set(vma_source_directory "${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/src")

    add_custom_target(Copy_vma_gpumemdumpvis 
        ALL DEPENDS VulkanMemoryAllocator 
        COMMAND ${CMAKE_COMMAND} 
            -E copy_directory 
                ${vma_source_directory}/tools/GpuMemDumpVis/
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/GpuMemDumpVis/
                
        VERBATIM
    )

endif()    
