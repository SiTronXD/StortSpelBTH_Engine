#pragma once

#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

// View/projection matrices and position
struct CameraBufferData
{
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec4 worldPosition = glm::vec4(glm::vec3(0.0f), 1.0f);
};

// Shadow map data
struct ShadowMapData
{
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec2 shadowMapSize = glm::vec2(0.0f);
    float shadowMapMinBias = 0.00001f;
    float shadowMapAngleBias = 0.001f;
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