#pragma once

#include <vector>
#include "../Graphics/MeshData.h"

struct Plane
{
	glm::vec3 pos;
	glm::vec3 normal;

	Plane(const glm::vec3& pos, const glm::vec3& normal)
	{
		this->pos = pos;
		this->normal = glm::normalize(normal);
	}
};

class BSPNode
{
private:
	std::vector<uint32_t> nodeIndices;

	Plane nodePlane;

	BSPNode* negativeChild;
	BSPNode* positiveChild;

	uint32_t depthLevel;

	float projectPointOnNormal(const Vertex& v, const Plane& plane);
	float projectPointOnNormal(const glm::vec3& p, const Plane& plane);
	
	bool isZero(const float& x);
	bool isLargerThanZero(const float& x);
	bool isLessThanZero(const float& x);

	bool inSameHalfSpace(const float& t0, const float& t1);
	bool isTriangleDegenerate(
		std::vector<Vertex>& vertices,
		const uint32_t& index0,
		const uint32_t& index1,
		const uint32_t& index2,
		glm::vec3& outputUnnormalizedNormal);
	bool isMeshConvex(
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices
	);
	bool foundTriangle(
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices, 
		uint32_t& triStartIndex,
		glm::vec3& outputUnnormalizedNormal);

public:
	BSPNode(const uint32_t& depthLevel);
	~BSPNode();

	void splitMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	void getMergedIndices(std::vector<uint32_t>& outputIndices);
	void traverseBackToFront(std::vector<uint32_t>& outputIndices, const glm::vec3& camPos);
	//void traverseFrontToBack(std::vector<uint32_t>& outputIndices, const glm::vec3& camPos);

	void getTreeDepth(uint32_t& value);
};