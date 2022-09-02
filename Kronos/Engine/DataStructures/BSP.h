#pragma once

#include "BSPNode.h"

class BSP
{
private:
	BSPNode* rootNode;

	void deleteRoot();

public:
	BSP();
	~BSP();

	void createFromMeshData(MeshData& meshData);
	void traverseTree(MeshData& meshData, const glm::vec3& camPos);

	uint32_t getTreeDepth();
};