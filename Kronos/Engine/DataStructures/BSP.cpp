#include "BSP.h"
#include "../Dev/Log.h"

void BSP::deleteRoot()
{
	delete this->rootNode;
	this->rootNode = nullptr;
}

BSP::BSP()
	: rootNode(nullptr)
{
}

BSP::~BSP()
{
	this->deleteRoot();
}

void BSP::createFromMeshData(MeshData& meshData)
{
	uint32_t numOriginalVerts = meshData.getVertices().size();
	uint32_t numOriginalInd = meshData.getIndices().size();

	// Create root node
	this->deleteRoot();
	this->rootNode = new BSPNode(0);

	// Make a copy of the mesh data
	std::vector<Vertex> vertices = meshData.getVertices();
	std::vector<uint32_t> indices = meshData.getIndices();

	// Split recursively
	this->rootNode->splitMesh(vertices, indices);

	// Get new indices
	std::vector<uint32_t> newIndices;
	newIndices.reserve(indices.size());
	this->rootNode->getMergedIndices(newIndices);

	// Write
	Log::write("");
	Log::write("Before BSP creation:");
	Log::write("Num vertices: " + std::to_string(numOriginalVerts));
	Log::write("Num indices: " + std::to_string(numOriginalInd));
	Log::write("Num triangles: " + std::to_string(numOriginalInd / 3) + "\n");
	Log::write("After BSP creation:");
	Log::write("Num vertices: " + std::to_string(vertices.size()));
	Log::write("Num indices: " + std::to_string(newIndices.size()));
	Log::write("Num triangles: " + std::to_string(newIndices.size() / 3) + "\n");
	Log::write("BSP tree depth: " + std::to_string(this->getTreeDepth()));

	// Apply 
	meshData.getVertices().assign(vertices.begin(), vertices.end());
	meshData.getIndices().assign(newIndices.begin(), newIndices.end());
}

void BSP::traverseTree(MeshData& meshData, const glm::vec3& camPos)
{
	// New indices for this frame
	meshData.getIndices().clear();

	// Fill new indices by traversing tree
	this->rootNode->traverseBackToFront(meshData.getIndices(), camPos);
}

uint32_t BSP::getTreeDepth()
{
	uint32_t treeDepth = 0;
	this->rootNode->getTreeDepth(treeDepth);

	return treeDepth;
}
