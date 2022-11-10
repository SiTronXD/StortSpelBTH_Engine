#pragma once

#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

// View/projection matrices and position
struct CameraBufferData
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec4 worldPosition;
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

    uint32_t padding0;
};

// Light data per light
struct LightBufferData
{
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 color;
};