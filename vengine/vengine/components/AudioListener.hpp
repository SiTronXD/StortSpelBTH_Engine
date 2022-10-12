#pragma once
#include <SFML/Audio.hpp>
#include "glm/glm.hpp"

struct AudioListener
{
	// Ranges from 0 - 100
	float volume;

	AudioListener(float volume = sf::Listener::getGlobalVolume())
	{
		this->volume = volume;
	}

	void setVolume(float volume)
	{
		this->volume = volume;
		sf::Listener::setGlobalVolume(volume);
	}
	float getVolume() const
	{
		return volume;
	}

	void setPosition(float x, float y, float z)
	{
		sf::Listener::setPosition(x, y, z);
	}
	void setPosition(const glm::vec3& position)
	{
		sf::Listener::setPosition(position.x, position.y, position.z);
	}

	void setOrientation(const glm::vec3& fwdDirection, const glm::vec3& upDirection)
	{
		sf::Listener::setDirection(fwdDirection.x, fwdDirection.y, fwdDirection.z);
		sf::Listener::setUpVector(upDirection.x, upDirection.y, upDirection.z);
	}
	void setFwdVector(const glm::vec3& fwdDirection)
	{
		sf::Listener::setDirection(fwdDirection.x, fwdDirection.y, fwdDirection.z);
	}
	void setUpVector(const glm::vec3& upDirection)
	{
		sf::Listener::setUpVector(upDirection.x, upDirection.y, upDirection.z);
	}
};