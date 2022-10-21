#pragma once

#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

// View projection matrices
struct UboViewProjection
{
    glm::mat4 projection;
    glm::mat4 view;
};