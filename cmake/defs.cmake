
# Sets the variable both in the current scope and the parents scope...
macro(set_parentScope VAR VAL) 
    set(${VAR} ${VAL})
    set(${VAR} ${${VAR}} PARENT_SCOPE)
endmacro()

macro(set_advanced VAR VAL)
    set_parentScope(${VAR} ${VAL})
    mark_as_advanced(${VAR})
endmacro()

function(setup_vengine_definitions)
    set_parentScope(BUILD_AS_LIBRARY "library"       )
    set_parentScope(BUILD_AS_EXECUTABLE "executable" )

    # Vengine Path Constants
    set_parentScope(VENGINE_ASSETS_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/vengine/vengine_assets       )
    set_parentScope(VENGINE_DEMO_ASSET_LINK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/demos/basic_demo/assets      )
    set_parentScope(VENGINE_DEPS_DIR            ${CMAKE_CURRENT_SOURCE_DIR}/deps         )
    set_parentScope(VENGINE_SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/vengine/vengine  )
    set_parentScope(VENGINE_TESTS_SOURCE_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/test_src     )
    set_parentScope(VENGINE_LIBS_DIR            ${VENGINE_DEPS_DIR}/lib                  )
    set_parentScope(VENGINE_INCLUDES_DIR        ${VENGINE_DEPS_DIR}/include              )

    # Paths used for Symbolic Link for assets
    set_parentScope(VENGINE_ASSET_LINK_SRC "${VENGINE_ASSETS_DIR}"              )
    set_parentScope(VENGINE_DEMO_ASSET_LINK_SRC "${VENGINE_DEMO_ASSET_LINK_DIR}"              )
    if(${WIN32}) 
    set_parentScope(VENGINE_ASSET_LINK_DST "${CMAKE_CURRENT_BINARY_DIR}/Debug/assets" )
    else()
    set_parentScope(VENGINE_ASSET_LINK_DST "${CMAKE_CURRENT_BINARY_DIR}/vengine_assets" )
    set_parentScope(VENGINE_DEMO_ASSET_LINK_DST "${CMAKE_CURRENT_BINARY_DIR}/assets" )
    endif()

    # Directory Structure for assets and deps folder
    set_parentScope(VENGINE_AS_LIB_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/VENGINE_AS_LIB    )
    set_parentScope(VENGINE_AS_LIB_ASSETS ${VENGINE_AS_LIB_FOLDER}/assets               )
    set_parentScope(VENGINE_AS_LIB_MODELS ${VENGINE_AS_LIB_ASSETS}/models               )
    set_parentScope(VENGINE_AS_LIB_SHADERS ${VENGINE_AS_LIB_ASSETS}/shaders             )
    set_parentScope(VENGINE_AS_LIB_TEXTURES ${VENGINE_AS_LIB_ASSETS}/textures           )
    set_parentScope(VENGINE_AS_LIB_DEPS ${VENGINE_AS_LIB_FOLDER}/deps                   )
    set_parentScope(VENGINE_AS_LIB_INCLUDE ${VENGINE_AS_LIB_FOLDER}/deps/include        )
    set_parentScope(VENGINE_AS_LIB_LIB ${VENGINE_AS_LIB_FOLDER}/deps/lib                ) 
    mark_as_advanced(VENGINE_AS_LIB_MODELS VENGINE_AS_LIB_SHADERS
        VENGINE_AS_LIB_TEXTURES VENGINE_AS_LIB_DEPS VENGINE_AS_LIB_INCLUDE)   

    # Default values 
    set_advanced(VENGINE_DFT_ASSETS_INSTALL_DIR ${VENGINE_AS_LIB_ASSETS})
    #set(VENGINE_DFT_ASSETS_INSTALL_DIR ${VENGINE_AS_LIB_ASSETS} )
    mark_as_advanced(VENGINE_DFT_ASSETS_INSTALL_DIR)
endfunction()

#Location Must be Relative to CMakeLists (root) location... 
function(update_vengine_assets_location newPath)
    # Vengine Path Constants
    set_parentScope(VENGINE_ASSETS_DIR          ${update_vengine_assets_location}/vengine/vengine_assets       )

    # Paths used for Symbolic Link for assets
    set_parentScope(VENGINE_ASSET_LINK_SRC "${VENGINE_ASSETS_DIR}"              )
    set_parentScope(VENGINE_ASSET_LINK_DST "${CMAKE_CURRENT_BINARY_DIR}/assets" )

    # Directory Structure for assets and deps folder
    set_parentScope(VENGINE_AS_LIB_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/VENGINE_AS_LIB    )
    set_parentScope(VENGINE_AS_LIB_ASSETS ${VENGINE_AS_LIB_FOLDER}/assets               )
    set_parentScope(VENGINE_AS_LIB_MODELS ${VENGINE_AS_LIB_ASSETS}/models               )
    set_parentScope(VENGINE_AS_LIB_SHADERS ${VENGINE_AS_LIB_ASSETS}/shaders             )
    set_parentScope(VENGINE_AS_LIB_TEXTURES ${VENGINE_AS_LIB_ASSETS}/textures           )
    set_parentScope(VENGINE_AS_LIB_DEPS ${VENGINE_AS_LIB_FOLDER}/deps                   )
    set_parentScope(VENGINE_AS_LIB_INCLUDE ${VENGINE_AS_LIB_FOLDER}/deps/include        )
    set_parentScope(VENGINE_AS_LIB_LIB ${VENGINE_AS_LIB_FOLDER}/deps/lib                ) 
    mark_as_advanced(${VENGINE_AS_LIB_MODELS} ${VENGINE_AS_LIB_SHADERS} 
        ${VENGINE_AS_LIB_TEXTURES} ${VENGINE_AS_LIB_DEPS} ${VENGINE_AS_LIB_INCLUDE})   
endfunction()