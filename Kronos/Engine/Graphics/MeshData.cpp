#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj.h>

#include "MeshData.h"

MeshData::MeshData()
{
}

MeshData::~MeshData()
{
}

void MeshData::create(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	this->vertices.reserve(vertices.size());
	this->indices.reserve(indices.size());

	this->vertices.assign(vertices.begin(), vertices.end());
	this->indices.assign(indices.begin(), indices.end());
}

void MeshData::loadOBJ(const std::string& filePath)
{
	// Load model (assume triangulation)
	fastObjMesh* loadedObj = fast_obj_read(filePath.c_str());

	// Positions
	this->vertices.resize(loadedObj->index_count);
	for (unsigned int i = 0; i < loadedObj->position_count; ++i)
	{
		Vertex& v = this->vertices[i];
		v.pos.x = loadedObj->positions[i * 3 + 0];
		v.pos.y = loadedObj->positions[i * 3 + 1];
		v.pos.z = loadedObj->positions[i * 3 + 2];

		v.color.x = 0.5f;
	}

	// Indices
	this->indices.resize(loadedObj->index_count);
	for (unsigned int i = 0; i < loadedObj->index_count; ++i)
	{
		this->indices[i] = loadedObj->indices[i].p;
	}

	// Visualize normals as colors
	for (size_t i = 0; i < this->indices.size(); i += 3)
	{
		Vertex& v0 = this->vertices[this->indices[i + 0]];
		Vertex& v1 = this->vertices[this->indices[i + 1]];
		Vertex& v2 = this->vertices[this->indices[i + 2]];

		const glm::vec3 edge0 = v1.pos - v0.pos;
		const glm::vec3 edge1 = v2.pos - v0.pos;

		glm::vec3 normal = glm::cross(edge0, edge1);
		normal = glm::normalize(normal);

		v0.color += normal;
		v1.color += normal;
		v2.color += normal;
	}

	// Normalize smooth normals
	for (size_t i = 0; i < this->vertices.size(); ++i)
	{
		Vertex& v = this->vertices[i];
		if (glm::dot(v.color, v.color) >= 0.01f)
			v.color = glm::normalize(v.color);

		// Single color
		// v.color = glm::vec3(0.5f, 0.0f, 0.0f);
	}

	// Destroy loaded obj model
	fast_obj_destroy(loadedObj);
}
