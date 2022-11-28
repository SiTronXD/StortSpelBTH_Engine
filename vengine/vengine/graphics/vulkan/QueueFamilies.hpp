#pragma once

#include "../TempPCH.hpp"

struct QueueFamilyIndices
{
	// Note! Both the GraphicsFamily and PresentationFamily queue might be the same QueueFamily!
	// - Presentation is actually a Feature of a Queue Family, rather than being a seperate Queue Family Type...!
	int32_t graphicsFamily = -1;
	int32_t presentationFamily = -1;
	int32_t computeFamily = -1;

	// Check if Queue Families are valid... (Invalid Families will have -1 as value...)
	[[nodiscard]] bool isValid() const {
		return graphicsFamily >= 0 && presentationFamily >= 0 && computeFamily >= 0;
	}
};

class QueueFamilies
{
private:
	QueueFamilyIndices indices{};
	vk::Queue graphicsQueue{};
	vk::Queue presentationQueue{};
	vk::Queue computeQueue{};

public:
	QueueFamilies();
	~QueueFamilies();

	void setGraphicsQueue(const vk::Queue& queue);
	void setPresentQueue(const vk::Queue& queue);
	void setComputeQueue(const vk::Queue& queue);

	inline QueueFamilyIndices& getIndices() { return this->indices; }
	inline vk::Queue& getGraphicsQueue() { return this->graphicsQueue; }
	inline vk::Queue& getPresentQueue() { return this->presentationQueue; }
	inline vk::Queue& getComputeQueue() { return this->computeQueue; }
	inline const int32_t& getGraphicsIndex() { return this->indices.graphicsFamily; }
	inline const int32_t& getPresentIndex() { return this->indices.presentationFamily; }
	inline const int32_t& getComputeIndex() { return this->indices.computeFamily; }
};