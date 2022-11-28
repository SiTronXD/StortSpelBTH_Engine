#include "pch.h"
#include "QueueFamilies.hpp"

QueueFamilies::QueueFamilies()
{ }

QueueFamilies::~QueueFamilies()
{ }

void QueueFamilies::setGraphicsQueue(const vk::Queue& queue)
{
	this->graphicsQueue = queue;
}

void QueueFamilies::setPresentQueue(const vk::Queue& queue)
{
	this->presentationQueue = queue;
}

void QueueFamilies::setComputeQueue(const vk::Queue& queue)
{
	this->computeQueue = queue;
}