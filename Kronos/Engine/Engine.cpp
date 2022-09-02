#include "Engine.h"
#include "Application/Time.h"
#include "Application/Input.h"
#include "Graphics/Mesh.h"
#include "DataStructures/BSP.h"

/*#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>*/

/*
std::vector<Vertex> quadsVertices =
{
	{{  0.5f,  0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
	{{ -0.5f,  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
	{{ -0.5f,  0.0f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }},
	{{  0.5f,  0.0f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},

	{{  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
	{{ -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
	{{ -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }},
	{{  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},
};
*/

std::vector<Vertex> quadsVertices =
{
	{{  0.5f,  0.0f, -0.5f }, { 0.5f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
	{{ -0.5f,  0.0f, -0.5f }, { 0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
	{{ -0.5f,  0.0f,  0.5f }, { 0.5f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
	{{  0.5f,  0.0f,  0.5f }, { 0.5f, 0.0f, 0.0f }, { 1.0f, 1.0f }},

	{{  0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
	{{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
	{{ -0.5f, -0.5f,  0.5f }, { 0.5f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
	{{  0.5f, -0.5f,  0.5f }, { 0.5f, 0.0f, 0.0f }, { 1.0f, 1.0f }},
};
std::vector<uint32_t> quadsIndices =
{
	4, 5, 6,
	6, 7, 4,

	0, 1, 2,
	2, 3, 0
};

Engine::Engine()
{
}

Engine::~Engine()
{
}

void Engine::init()
{
	
}
