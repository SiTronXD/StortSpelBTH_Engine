#pragma once

#include "glm/glm.hpp"

struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 color;

    // Cascade settings
    float cascadeSizes[3] =
    {
        20.0f,
        100.0f,
        500.0f
    };
    float cascadeDepthScale = 1.0f;

    bool cascadeVisualization = false;

    // Shadow map settings
    float shadowMapMinBias = 0.0001f;
    float shadowMapAngleBias = 0.0015f;
};