#pragma once

#include <iostream>
///#define GLFW_INCLUDE_VULKAN  <-- Include this in project settings or CmakeList...!
//#include <GLFW/glfw3.h>                 //! Includes GLFW after preprocessor GLFW_INCLUDE_VULKAN to include vulkan!
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

//#define GLM_FORCE_RADIANS              //! Forces GLM to use Radians instead of Degrees...
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE     //! Forces Vulkan's Depth buffer to go from 0 to 1, rather than -1 to 1
#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"

void section2_setupAndCompatibilityTest();

