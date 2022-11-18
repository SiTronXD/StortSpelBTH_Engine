#pragma once

#include "glm/glm.hpp"

struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 color;

    // Cascade settings
    float cascadeSize0 = 0.05f;
    float cascadeSize1 = 0.1f;
    float cascadeSize2 = 0.5f;
    float cascadeDepthScale = 1.0f;

    bool cascadeVisualization = false;

    // Shadow map settings
    float shadowMapMinBias = 0.0001f;
    float shadowMapAngleBias = 0.0015f;
};