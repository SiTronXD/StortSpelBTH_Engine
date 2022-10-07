#pragma once

#include "MeshData.hpp"

class MeshDataInfo
{
public:
	static size_t getVertexSize(
		const AvailableVertexData& availableVertexData);
	static size_t getVertexSize(
		const MeshData& meshData);
};