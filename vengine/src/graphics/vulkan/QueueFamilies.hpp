#pragma once

#include "../TempPCH.hpp"

class QueueFamilies
{
private:
	QueueFamilyIndices indices{};
	vk::Queue graphicsQueue{};
	vk::Queue presentationQueue{};

public:
	QueueFamilies();
	~QueueFamilies();

	void setGraphicsQueue(const vk::Queue& queue);
	void setPresentQueue(const vk::Queue& queue);

	inline QueueFamilyIndices& getIndices() { return this->indices; }
	inline vk::Queue& getGraphicsQueue() { return this->graphicsQueue; }
	inline vk::Queue& getPresentQueue() { return this->presentationQueue; }
	inline const int32_t& getGraphicsIndex() { return this->indices.graphicsFamily; }
	inline const int32_t& getPresentIndex() { return this->indices.presentationFamily; }
};