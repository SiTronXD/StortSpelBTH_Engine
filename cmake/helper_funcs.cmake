
# Used to force an option, and being able to change it in CMakeLists without old cached valued being used
macro(force_option VARIABLE DESCRIPTION STATUS)
    option(${VARIABLE} ${DESCRIPTION} ${STATUS})
    set(${VARIABLE} ${STATUS} CACHE BOOL ${DESCRIPTION} FORCE) #PARENT_SCOPE
endmacro()


# Function to Create a symlink when caleld from one directory to another
function(create_assets_symlink_for_client FROM_LOCATION TO_LOCATION)
    
    create_symlink_now(${VENGINE_INSTALL_ASSETS_PATH}/assets ${VENGINE_SYMLINK_TO_ASSETS_PATH}/assets)
endfunction()

# Function to Create a symlink when caleld from one directory to another
function(create_symlink_now FROM_LOCATION TO_LOCATION)
    
    if(NOT EXISTS "${TO_LOCATION}")        
    
        execute_process( 
            COMMAND ${CMAKE_COMMAND} 
                -E create_symlink ${FROM_LOCATION} ${TO_LOCATION} 
            RESULT_VARIABLE SYMLINK_RESULT
        )
        if(${SYMLINK_RESULT} MATCHES 0)
            message("Creating symbolic link ${FROM_LOCATION} => ${TO_LOCATION}")
        else()
            message("WARNING! Failed creating symlink: ${FROM_LOCATION} => ${TO_LOCATION}")
        endif()
    endif()

endfunction()

# Function to Create a symlink on Build from one directory to another
function(create_symlink_buildtime FROM_LOCATION TO_LOCATION)
    message( "start: create symlinks: ${FROM_LOCATION} => ${TO_LOCATION}" )         
    add_custom_command( 
        OUTPUT ${TO_LOCATION}
        POST_BUILD COMMAND ${CMAKE_COMMAND}
            -E create_symlink ${FROM_LOCATION} ${TO_LOCATION} 
        #DEPENDS ${FROM_LOCATION}
        COMMENT "Creating symbolic link ${FROM_LOCATION} => ${TO_LOCATION}"
         )

endfunction()

# Function to copy files 
function(copy_files FILE_ORIGIN DESTINATION)

    get_filename_component(FILENAME ${FILE_ORIGIN} NAME)
    set(COPY_FROM ${FILE_ORIGIN})
    set(COPY_TO ${DESTINATION}/${FILENAME})

    get_filename_component(CURRENT_OUT_PATH ${COPY_TO} DIRECTORY)
    file(MAKE_DIRECTORY ${CURRENT_OUT_PATH})    

    configure_file(${COPY_FROM} ${COPY_TO} COPYONLY )

endfunction(copy_files)

function(copy_assets_structure)
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_FOLDER}) # Create folder that contains everything a user needs
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_ASSETS}) # Create where to look for assets
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_MODELS}) # Create where to look for models
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_SHADERS}) # Create where to look for shaders
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_TEXTURES}) # Create where to look for textures
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_DEPS}) # Create main folder for libs and include
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_INCLUDE}) # Create main folder for include
    file(MAKE_DIRECTORY ${VENGINE_AS_LIB_LIB}) # Create main folder for libs

    #Copy necessary files 
    configure_file(${VENGINE_ASSETS_DIR}/config.cfg ${VENGINE_AS_LIB_ASSETS}/config.cfg COPYONLY )
    configure_file(${VENGINE_ASSETS_DIR}/textures/missing_texture.jpg ${VENGINE_AS_LIB_TEXTURES}/missing_texture.jpg COPYONLY )
    
    file(GLOB files_in_shader_dir CONFIGURE_DEPENDS
        ${VENGINE_ASSETS_DIR}/shaders/*
    )
    foreach(shader_file IN LISTS files_in_shader_dir)
        get_filename_component(NAMEONLY shader_file NAME)
        copy_files(${shader_file} ${VENGINE_AS_LIB_SHADERS} )    
    endforeach()
endfunction()

function(install_asset_dir INSTALL_LOCATION)
    
    if(NOT EXISTS "${INSTALL_LOCATION}/assets")
        message("Installing assets dir to location '${INSTALL_LOCATION}'!")
        file(COPY ${VENGINE_AS_LIB_ASSETS} DESTINATION ${INSTALL_LOCATION})
    endif()
    
endfunction()

function(install_vengine_helpers INSTALL_LOCATION)
    if(NOT EXISTS "${INSTALL_LOCATION}/vengine_helpers.cmake")
        message("Installing vengine_helpers.cmake to location '${INSTALL_LOCATION}'!")
        #file(COPY ${CMAKE_SOURCE_DIR}/cmake/vengine_helpers.cmake DESTINATION ${INSTALL_LOCATION})
        file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vengine_helpers.cmake DESTINATION ${INSTALL_LOCATION})
    endif()
    
endfunction()


macro(vengine_find_module LIBRARY_NAME CMAKELISTS_PATH REAL_LIB_NAME)
    message(VERBOSE "Beginning vengine_find for ${LIBRARY_NAME}")
    find_package(${LIBRARY_NAME} MODULE QUIET)
    message(VERBOSE "vengine_find(${LIBRARY_NAME}) resulted in: " ${vengine_vma_FOUND})
    
    if(NOT ${${LIBRARY_NAME}_FOUND}) 
        message("vengine_find did not find ${LIBRARY_NAME}, building from source!")
        add_subdirectory(${CMAKELISTS_PATH})
        
        set(${LIBRARY_NAME} ${REAL_LIB_NAME})
    else()
        set(${LIBRARY_NAME} ${LIBRARY_NAME}::${LIBRARY_NAME})
    endif()

    message(VERBOSE "leaving vengine_find(${LIBRARY_NAME})")

endmacro()

macro(vengine_find_pkg LIBRARY_NAME CMAKELISTS_PATH)
    message(VERBOSE "Beginning vengine_cmake_find for ${LIBRARY_NAME}")
    find_package(${LIBRARY_NAME} QUIET)
    message(VERBOSE "vengine_cmake_find(${LIBRARY_NAME}) resulted in: " ${vengine_vma_FOUND})
    
    if(NOT ${${LIBRARY_NAME}_FOUND}) 
        message("vengine_cmake_find did not find ${LIBRARY_NAME}, building from source!")
        add_subdirectory(${CMAKELISTS_PATH})    
    else()

    endif()

    message(VERBOSE "leaving vengine_cmake_find(${LIBRARY_NAME})")

endmacro()