#include "Mesh.h"

#include "Renderer.h"

Mesh::Mesh(Renderer& renderer)
	: vertexBuffer(renderer), 
	indexBuffer(renderer),
	numIndices(0)
{
}

Mesh::~Mesh()
{
}

void Mesh::createMesh(
	MeshData& meshData,
	bool cpuWriteToIndexBuffer)
{
	this->vertexBuffer.createVertexBuffer(meshData.getVertices());
	this->indexBuffer.createIndexBuffer(meshData.getIndices(), cpuWriteToIndexBuffer);

	this->numIndices = meshData.getIndices().size();
}

void Mesh::cleanup()
{
	this->indexBuffer.cleanup();
	this->vertexBuffer.cleanup();
}
