#pragma once

#include "TempPCH.hpp"

class Device;

class Texture
{
private:
public:
	Texture();
	~Texture();

	static vk::ImageView createImageView(
		Device& device,
		vk::Image image, 
		vk::Format format, 
		vk::ImageAspectFlags aspectFlags
	);
};