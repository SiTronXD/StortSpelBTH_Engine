#pragma once 
 #include "op_overload.hpp"

struct UIArea // 2D only
{
	glm::vec2 position;
	glm::vec2 dimension;
	Mouse clickButton = Mouse::LEFT;

	const bool isHovering() const
	{
		glm::vec2 diff = glm::abs(Input::getMouseUITranslated() - this->position);
		return diff.x <= this->dimension.x * 0.5f && diff.y <= this->dimension.y * 0.5f;
	}

	const bool isClicking() const
	{
		return this->isHovering() && Input::isMouseButtonPressed(clickButton);
	}
};