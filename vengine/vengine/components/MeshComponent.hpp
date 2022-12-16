#pragma once

#include "../graphics/MaterialData.hpp"

#define MAX_NUM_MESH_MATERIALS 8
struct MeshComponent
{
	int meshID;

	Material overrideMaterials[MAX_NUM_MESH_MATERIALS]{};
	uint32_t numOverrideMaterials = 0;

	bool receiveShadows = true;
};