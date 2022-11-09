#pragma once

#include "../graphics/MaterialData.hpp"

#define MAX_NUM_MESH_MATERIALS 16
struct MeshComponent
{
	int meshID;
    char filePath[64] = "ghost.obj";

	Material overrideMaterials[MAX_NUM_MESH_MATERIALS]{};
	uint32_t numOverrideMaterials = 0;
};