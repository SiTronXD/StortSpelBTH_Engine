#include "pch.h"
#include "MeshDataModifier.hpp"
#include "../dev/Log.hpp"

void MeshDataModifier::smoothNormals(MeshData& meshData)
{
	if (meshData.indicies.size() % 3 != 0)
	{
		Log::warning(
			"MeshDataModifier::smoothNormals: Meshdata does not have an index count divisible by 3. Num indices: " + std::to_string(meshData.indicies.size()));
	}

	// Clear normals
	size_t numNormals = meshData.vertexStreams.normals.size();
	for (size_t i = 0; i < numNormals; ++i)
	{
		meshData.vertexStreams.normals[i] = glm::vec3(0.0f);
	}

	// Loop through all indices
	size_t numIndices = meshData.indicies.size() - (meshData.indicies.size() % 3);
	for (size_t i = 0; i < numIndices; i += 3)
	{
		const uint32_t index0 = meshData.indicies[i + 0];
		const uint32_t index1 = meshData.indicies[i + 1];
		const uint32_t index2 = meshData.indicies[i + 2];

		const glm::vec3& pos0 = meshData.vertexStreams.positions[index0];
		const glm::vec3& pos1 = meshData.vertexStreams.positions[index1];
		const glm::vec3& pos2 = meshData.vertexStreams.positions[index2];

		// Create edges
		const glm::vec3 edge1 = pos1 - pos0;
		const glm::vec3 edge2 = pos2 - pos0;

		// Normal
		const glm::vec3 normal = 
			glm::normalize(glm::cross(edge1, edge2));

		// Add normal
		meshData.vertexStreams.normals[index0] += normal;
		meshData.vertexStreams.normals[index1] += normal;
		meshData.vertexStreams.normals[index2] += normal;
	}

	// Normalize added normals
	for (size_t i = 0; i < numNormals; ++i)
	{
		meshData.vertexStreams.normals[i] = 
			glm::normalize(meshData.vertexStreams.normals[i]);
	}
}

void MeshDataModifier::clearVertexStreams(MeshData& meshData)
{
	meshData.vertexStreams.positions.clear();
	meshData.vertexStreams.positions.shrink_to_fit();
	meshData.vertexStreams.normals.clear();
	meshData.vertexStreams.normals.shrink_to_fit();
	meshData.vertexStreams.colors.clear();
	meshData.vertexStreams.colors.shrink_to_fit();
	meshData.vertexStreams.texCoords.clear();
	meshData.vertexStreams.texCoords.shrink_to_fit();
	meshData.vertexStreams.boneWeights.clear();
	meshData.vertexStreams.boneWeights.shrink_to_fit();
	meshData.vertexStreams.boneIndices.clear();
	meshData.vertexStreams.boneIndices.shrink_to_fit();
}
