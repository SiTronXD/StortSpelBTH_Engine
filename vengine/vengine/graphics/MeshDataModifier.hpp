#pragma once

#include "MeshData.hpp"

class MeshDataModifier
{
private:
public:
	static void smoothNormals(MeshData& meshData);
	static void clearVertexStreams(MeshData& meshData);
};