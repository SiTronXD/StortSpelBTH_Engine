include(GNUInstallDirs)

file(GLOB TRACY_INITIATED "${CMAKE_CURRENT_SOURCE_DIR}/deps/tracy/*")
list(LENGTH TRACY_INITIATED TRACY_DIR_EMPTY) # TRACY_DIR_EMPTY == 0 if empty...

if(${VENGINE_TRACY_BUILD_PROFILER} AND 
    EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/tracy" OR NOT ("${TRACY_DIR_EMPTY}" EQUAL 0) )    #Add 'OR forceupdate' ... 
    
    set(tracy_source_directory "${CMAKE_CURRENT_SOURCE_DIR}/deps/tracy/src")
    set(tracy_binary_directory "${CMAKE_CURRENT_SOURCE_DIR}/deps/tracy/build")

    include(ProcessorCount)
    ProcessorCount(N)
    math(EXPR nrOfThreads "${N} - 1")
    if(${UNIX})

        # Build the profiler
        add_custom_command(
            OUTPUT ${tracy_source_directory}/profiler/build/unix/Tracy-release
            DEPENDS ${tracy_source_directory}
            #COMMAND make release -j6 -o"${tracy_binary_directory}/Tracy-release"
            #COMMAND make -j$$(($$(nproc)-1)) -o"${tracy_binary_directory}/Tracy-release"
            COMMAND make -j${N} -o"${tracy_binary_directory}/Tracy-release"
            WORKING_DIRECTORY ${tracy_source_directory}/profiler/build/unix        
            COMMENT "Building Tracy Profiler. (Using ${nrOfThreads} threads). this can take a while..."
            VERBATIM
        )
        # Make sure Tracy_profiler is built
        add_custom_target(Tracy_profiler ALL DEPENDS ${tracy_source_directory}/profiler/build/unix/Tracy-release)
        # Copy Profiler app to Tools directory
        add_custom_target(Copy_tracy_profiler 
            ALL DEPENDS Tracy_profiler 
            COMMAND ${CMAKE_COMMAND} 
                -E copy_if_different 
                ${tracy_source_directory}/profiler/build/unix/Tracy-release 
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/Tracy-release
            
            VERBATIM
        )        
    endif(${UNIX})
    # if(${WIN32}) #TODO: make this work on windows...
    #     message("Should not be here")
    #     # Build the profiler
    #     add_custom_command(
    #         OUTPUT ${tracy_source_directory}/profiler/build/unix/Tracy-release
    #         DEPENDS ${tracy_source_directory}            
    #         COMMAND ninja -o"${tracy_binary_directory}/Tracy-release"
    #         WORKING_DIRECTORY ${tracy_source_directory}/profiler/build/unix        
    #         VERBATIM
    #     )
    #         # Make sure Tracy_profiler is built
    #     add_custom_target(Tracy_profiler ALL DEPENDS ${tracy_source_directory}/profiler/build/unix/Tracy-release)
    #     # Copy Profiler app to Tools directory
    #     add_custom_target(Copy_tracy_profiler 
    #         ALL DEPENDS Tracy_profiler 
    #         COMMAND ${CMAKE_COMMAND} 
    #             -E copy_if_different 
    #             ${tracy_source_directory}/profiler/build/unix/Tracy-release 
    #             ${tracy_source_directory}/tools/Tracy-release
                
    #         VERBATIM
    #     )        
    # endif(${WIN32})

endif()    
