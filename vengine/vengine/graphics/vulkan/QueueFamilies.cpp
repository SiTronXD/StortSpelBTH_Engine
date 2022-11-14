#include "pch.h"

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