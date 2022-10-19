#version 460

#define FREQ_PER_FRAME 0

// Vertex data in world space
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform UboViewProjection
{
    mat4 projection;
    mat4 view;    
} uboViewProjection;

// Output data
layout(location = 0) out vec3 fragCol;

void main()
{
    gl_Position = 
        uboViewProjection.projection *
        uboViewProjection.view *
        vec4(pos, 1.0);

    fragCol = col;
}