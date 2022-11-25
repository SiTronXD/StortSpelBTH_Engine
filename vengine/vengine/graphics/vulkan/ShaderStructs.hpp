#pragma once

#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

struct ShadowPushConstantData
{
    glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
};

struct PushConstantData
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec4 tintColor = glm::vec4(0.0f);      // vec4(RGB, lerp alpha)
    glm::vec4 emissionColor = glm::vec4(0.0f);  // vec4(RGB, intensity)
    glm::vec4 settings = glm::vec4(0.0f);       // vec4(receiveShadows, 0, 0, 0)
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

// Shadow map data
#define MAX_NUM_CASCADES 4
struct ShadowMapData
{
    glm::mat4 viewProjection[MAX_NUM_CASCADES] = { glm::mat4(1.0f) };
    glm::vec2 shadowMapSize = glm::vec2(0.0f);
    float shadowMapMinBias = 0.0001f;
    float shadowMapAngleBias = 0.0015f;
    glm::vec4 cascadeFarPlanes = glm::vec4(0.0f);
    glm::uvec4 cascadeSettings = glm::uvec4(0.0f);
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