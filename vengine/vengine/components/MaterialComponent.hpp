#pragma once

struct MaterialComponent
{
	int materialID;
	char materialName[64];
	uint32_t textureID = 0;
};