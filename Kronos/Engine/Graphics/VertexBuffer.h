#pragma once

#include "../SMath.h"
#include "Buffer.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	// Describes data load rate throughout vertices
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	// Describes how to extract vertex attribute from vertex data in 
	// binding description
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		// Position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		// Color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		// Texture coordinates
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	static Vertex interpolateVertex(
		const Vertex& v0, 
		const Vertex& v1, 
		const float& t)
	{
		Vertex newVert{};
		newVert.pos = v0.pos + (v1.pos - v0.pos) * t;
		newVert.texCoord = v0.texCoord + (v1.texCoord - v0.texCoord) * t;
		newVert.color = v0.color + (v1.color - v0.color) * t;

		return newVert;
	}
};

class VertexBuffer : public Buffer
{
private:
public:
	VertexBuffer(Renderer& renderer);
	~VertexBuffer();

	void createVertexBuffer(const std::vector<Vertex>& vertices);
};