#version 460

#define FREQ_PER_FRAME 0

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform CameraBuffer
{
    mat4 projection;
    mat4 view;
    vec4 worldPos;
} cameraBuffer;

// Storage buffer
struct ParticleInfoData
{
    mat4 transform;
    vec4 life;  // vec4(currentLifeTime, 0.0f, 0.0f, 0.0f)
    vec2 startSize;
    vec2 endSize;
    vec4 currentColor;
    vec4 currentVelocity;
    vec4 acceleration;
    uvec4 indices; // vec4(randomState, emitterIndex, 0, 0)
};
layout(std140, set = FREQ_PER_FRAME, binding = 1) readonly buffer ParticleInfosBuffer
{
    ParticleInfoData infos[];
} particles;

// Output data
layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragTintCol;

void main()
{
    uint localVertexIndex = gl_VertexIndex % 6;

    // These calculations are done to avoid array indexing
    vec2 position = vec2(0.0f, 0.0f);
    position.x = localVertexIndex < 3 ? 
        float(int(localVertexIndex / 2) * 2 - 1) : // 0, 1, 2
        float(localVertexIndex % 2 == 1 ? 1 : -1); // 3, 4, 5
    position.y = localVertexIndex < 3 ?
        float(localVertexIndex % 2 == 0 ? 1 : -1) : // 0, 1, 2
        float(int((5 - localVertexIndex) / 2) * 2 - 1); // 3, 4, 5

    // Arrays to avoid indexing into:
    /*const vec2 positions[6] = vec2[](
        vec2(-1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0)
    );
    vec2 uvs[6] = vec2[](
        vec2(0.0f, 0.0f),
        vec2(0.0f, 1.0f),
        vec2(1.0f, 0.0f),

        vec2(1.0f, 0.0f),
        vec2(0.0f, 1.0f),
        vec2(1.0f, 1.0f)
    );*/

    
    ParticleInfoData particle = 
        particles.infos[gl_InstanceIndex];

    // Position
    gl_Position = 
        cameraBuffer.projection * 
        cameraBuffer.view * 
        particle.transform *
        vec4(position, 0.0f, 1.0f);

    // UV coordinates
    fragUV = vec2(position.x, -position.y) * 0.5f + vec2(0.5f);

    // Tint color
    fragTintCol = particle.currentColor;
}