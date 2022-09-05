# Setup Vulkan SPIR-V Shader Compilation ================================== 
# stolen from:(https://www.reddit.com/r/vulkan/comments/kbaxlz/what_is_your_workflow_when_compiling_shader_files/ and https://stackoverflow.com/questions/57478571/why-wont-cmake-build-my-vulkan-spirv-shaders)
# If this does not work, use compile_shaders.sh script in shader folder...
# Function to Compile GLSL to SPIR-V
function(add_shader TARGET SHADER SHADER_SOURCE_DIR SHADER_BINARY_DIR)
    find_program(GLSLC glslc)

    set(SHADER_PATH ${SHADER_SOURCE_DIR}/${SHADER})
    set(SHADER_OUT_PATH ${SHADER_BINARY_DIR}/${SHADER}.spv)

    # Add a custom command to compile GLSL to SPIR-V.
    get_filename_component(CURRENT_OUT_PATH ${SHADER_OUT_PATH} DIRECTORY)
    file(MAKE_DIRECTORY ${CURRENT_OUT_PATH})

    add_custom_command(
           OUTPUT ${SHADER_OUT_PATH}
           COMMAND ${GLSLC} -o ${SHADER_OUT_PATH} ${SHADER_PATH}
           DEPENDS ${SHADER_PATH}
           IMPLICIT_DEPENDS CXX ${SHADER_PATH}
           VERBATIM)
    
    # Make sure our build depends on this output.
    set_source_files_properties(${SHADER_OUT_PATH} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${SHADER_OUT_PATH})

endfunction(add_shader)

function(build_shaders TARGET SHADER_INPUT_DIR SHADER_OUTPUT_LOCATION)
    # Compiling Shaders
    set(SHADER_SOURCE_DIR ${SHADER_INPUT_DIR}/shaders)
    set(SHADER_BINARY_DIR ${SHADER_OUTPUT_LOCATION}/shaders)

    file(GLOB SHADERS
    ${SHADER_SOURCE_DIR}/*.vert
    ${SHADER_SOURCE_DIR}/*.frag
    ${SHADER_SOURCE_DIR}/*.comp
    ${SHADER_SOURCE_DIR}/*.geom
    ${SHADER_SOURCE_DIR}/*.tesc
    ${SHADER_SOURCE_DIR}/*.tese
    ${SHADER_SOURCE_DIR}/*.mesh
    ${SHADER_SOURCE_DIR}/*.task
    ${SHADER_SOURCE_DIR}/*.rgen
    ${SHADER_SOURCE_DIR}/*.rchit
    ${SHADER_SOURCE_DIR}/*.rmiss)

    foreach(shader IN LISTS SHADERS)
        get_filename_component(FILENAME ${shader} NAME)
        add_shader(${TARGET} ${FILENAME} ${SHADER_SOURCE_DIR} ${SHADER_BINARY_DIR})  
    endforeach()
endfunction()

#EndOf Vulkan Shader Compilation     ==================================
