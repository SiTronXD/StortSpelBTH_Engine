#pragma once

#include "MeshData.hpp"

class MeshDataInfo
{
public:
	static bool areStreamsValid(
		const VertexStreams& vertexStreams);
	static size_t getVertexSize(
		const VertexStreams& vertexStreams);
};