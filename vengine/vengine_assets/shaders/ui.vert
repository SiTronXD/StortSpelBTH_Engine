#version 460

// Vertex data
layout(location = 0) in vec3 pos;

// Output data
layout(location = 0) out vec2 fragUV;

void main()
{
    gl_Position = vec4(pos, 1.0);

    const vec2 uvs[6] = vec2[](
        vec2(0.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    fragUV = uvs[gl_VertexIndex % 6];
}