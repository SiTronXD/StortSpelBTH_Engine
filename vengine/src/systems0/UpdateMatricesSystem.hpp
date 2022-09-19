#pragma once

#include "System.hpp"
#include "../Components/Transform.hpp"

class UpdateMatricesSystem : public System 
{
private:

public:
	bool update(entt::registry& reg, float deltaTime) final
	{
		auto tView = reg.view<Transform>();

		tView.each([](Transform& transform)
		{
			transform.matrix = glm::translate(glm::mat4(1.0f), transform.position) *
				glm::scale(glm::mat4(1.0f), transform.scale) *
				glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		});

		return false;
	}
};