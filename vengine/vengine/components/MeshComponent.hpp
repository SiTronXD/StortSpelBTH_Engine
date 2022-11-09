#pragma once

#include "../graphics/MaterialData.hpp"

struct MeshComponent
{
	int meshID;
    char filePath[64] = "ghost.obj";

	Material overrideMaterials[16]{};
	uint32_t numOverrideMaterials = 0;
};