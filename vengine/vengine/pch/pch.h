#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <optional>
#include <fstream>
#include <chrono>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <set>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include "backends/imgui_impl_vulkan.h"          
#include "stb_image.h"

#include "../dev/Log.hpp"
#include "../application/Input.hpp"
#include "../application/Time.hpp"
#include "../dev/StringHelper.hpp"
#include "../graphics/StringAlignment.h"
#include "../graphics/vulkan/VulkanInstance.hpp"
#include "../graphics/vulkan/PhysicalDevice.hpp"
#include "../graphics/vulkan/Device.hpp"
#include "../graphics/vulkan/QueueFamilies.hpp"
#include "../graphics/vulkan/VulkanValidation.hpp"
#include "../graphics/vulkan/VulkanDbg.hpp"

#include "../components/Transform.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/Camera.hpp"
#include "../components/Script.hpp"
#include "../components/Collider.h"
#include "../components/Rigidbody.h"
#include "../components/AnimationComponent.hpp"
#include "../components/AudioSource.h"
#include "../components/UIArea.hpp"
#include "../components/BTAgent.hpp"
#include "../components/BTComponent.hpp"
#include "../components/FSMAgentComponent.hpp"
#include "../components/AmbientLight.hpp"
#include "../components/DirectionalLight.hpp"
#include "../components/PointLight.hpp"
#include "../components/Spotlight.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"