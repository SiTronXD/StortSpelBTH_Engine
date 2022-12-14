#pragma once

#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

struct PushConstantData
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec4 tintColor = glm::vec4(0.0f);      // vec4(RGB, lerp alpha)
    glm::vec4 emissionColor = glm::vec4(0.0f);  // vec4(RGB, intensity)
    glm::vec4 settings = glm::vec4(0.0f);       // vec4(receiveShadows, fogStart, fogAbsorbtion, 0)
    glm::vec4 tiling = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // vec4(offsetX, offsetY, scaleX, scaleY)
};

struct BloomPushConstantData
{
    // vec4(resolutionX, resolutionY, 0, upsampleWeight)
    glm::vec4 data = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
};

struct BloomSettingsBufferData
{
    glm::vec4 strength = glm::vec4(0.04f);
};

// View/projection matrices and position
struct CameraBufferData
{
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec4 worldPosition = glm::vec4(glm::vec3(0.0f), 1.0f);
};

// Data for all particles
struct GlobalParticleBufferData
{
    glm::vec2 padding = glm::vec3(0.0f);
    float deltaTime = 1.0f;
    uint32_t numParticles = 0;
};

// Per particle info
struct ParticleInfo
{
    glm::mat4 transformMatrix = glm::mat4(1.0f);
    glm::vec4 life = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // vec4(currentLifeTime, 0.0f, 0.0f, 0.0f)
    glm::vec2 startSize = glm::vec2(1.0f);
    glm::vec2 endSize = glm::vec2(1.0f);
    glm::vec4 currentColor = glm::vec4(1.0f);
    glm::vec4 currentVelocity = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 acceleration = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    glm::uvec4 indices = glm::uvec4(0u);    // vec4(randomState, emitterIndex, 0, 0)
};

// Per particle system info
struct ParticleEmitterInfo
{
    // Spawn volume
    glm::vec3 conePos;
    float coneDiskRadius;
    glm::vec3 coneDir;
    float tanTheta;
    glm::vec3 coneNormal;
    uint32_t shouldRespawn;
    glm::vec4 settings = glm::vec4(0.0f); // vec4(spawnRate, velocityStrength, maxLifeTime, 0.0f)
    glm::vec4 startColor = glm::vec4(1.0f);
    glm::vec4 endColor = glm::vec4(1.0f);
};

#define MAX_NUM_CASCADES 4

struct ShadowPushConstantData
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
};

// View/projection matrices for shadow map rendering
struct ShadowMapCameraBufferData
{
    glm::mat4 viewProjection[MAX_NUM_CASCADES] = { glm::mat4(1.0f) };
};

// Shadow map data
struct ShadowMapData
{
    glm::mat4 viewProjection[MAX_NUM_CASCADES] = { glm::mat4(1.0f) };
    glm::vec2 shadowMapSize = glm::vec2(0.0f);
    float shadowMapMinBias = 0.0001f;
    float shadowMapAngleBias = 0.0015f;
    glm::uvec4 cascadeSettings = glm::uvec4(0u);
};

// Offset indicies into the light buffer
struct AllLightsInfo
{
    // [0, ambientLightsEndIndex)
    uint32_t ambientLightsEndIndex;

    // [ambientLightsEndIndex, directionalLightsEndInde)
    uint32_t directionalLightsEndIndex;

    // [directionalLightsEndIndex, pointLightsEndIndex)
    uint32_t pointLightsEndIndex; 

    // [pointLightsEndIndex, spotlightsEndIndex)
    uint32_t spotlightsEndIndex;
};

// Light data per light
struct LightBufferData
{
    glm::vec4 position;
    glm::vec4 direction; // vec4(x, y, z, cos(angle / 2))
    glm::vec4 color;
};