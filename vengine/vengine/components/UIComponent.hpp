#pragma once

struct UIComponent // 2D only
{
	glm::ivec2 position;
	glm::ivec2 dimension;

	const bool isHovering() const
	{
		glm::ivec2 diff = glm::abs(Input::getMouseUITranslated() - this->position);
		return diff.x <= (this->dimension.x >> 1) && diff.y <= (this->dimension.y >> 1); // (>> 1) == (/ 2)
	}

	const bool isClicking() const
	{
		return this->isHovering() && Input::isMouseButtonPressed(Mouse::LEFT);
	}
};