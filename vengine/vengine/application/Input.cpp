#include "pch.h"
#include "Input.hpp"
#include "../graphics/ResTranslator.hpp"

std::map<Keys, bool> Input::keyDown;
std::map<Keys, bool> Input::lastKeyDown;
std::map<Mouse, bool> Input::mouseButtonDown;
std::map<Mouse, bool> Input::lastmouseButtonDown;

int Input::cursorX = 0;
int Input::cursorY = 0;
int Input::deltaCursorX = 0;
int Input::deltaCursorY = 0;
int Input::requestedMouseX = 0;
int Input::requestedMouseY = 0;

int Input::deltaScrollWheel = 0;

bool Input::shouldSetMousePos = false;
bool Input::shouldHideCursor = false;
bool Input::lastShouldHideCursor = false;

void Input::setDeltaCursor(
	const int& deltaCursorX, 
	const int& deltaCursorY)
{
	// Update delta
	Input::deltaCursorX = deltaCursorX;
	Input::deltaCursorY = deltaCursorY;
}

void Input::setDeltaScrollWheel(const int& deltaScrollWheel)
{
	Input::deltaScrollWheel = deltaScrollWheel;
}

void Input::setCursor(const int& newCursorX, const int& newCursorY)
{
	// Update current position
	Input::cursorX = newCursorX;
	Input::cursorY = newCursorY;
}

void Input::update()
{
	// Last keys
	for (auto const& [key, val] : Input::keyDown)
	{
		Input::lastKeyDown[key] = val;
	}

	// Last mouse buttons
	for (auto const& [mouse, val] : Input::mouseButtonDown)
	{
		Input::lastmouseButtonDown[mouse] = val;
	}

	// Last should hide cursor
	Input::lastShouldHideCursor = Input::shouldHideCursor;
}

void Input::setKey(const Keys& keyCode, const bool& value)
{
	// Set value
	if (Input::keyDown.count(keyCode))
	{
		Input::keyDown[keyCode] = value;
	}
	// Insert new element
	else
	{
		// Current
		Input::keyDown.insert(
			std::pair<Keys, bool>(
				keyCode,
				value
			)
		);

		// Last
		Input::lastKeyDown.insert(
			std::pair<Keys, bool>(
				keyCode,
				!value
			)
		);
	}
}

void Input::setMouseButton(const Mouse& mouseButtonCode, const bool& value)
{
	// Set value
	if (Input::mouseButtonDown.count(mouseButtonCode))
	{
		Input::mouseButtonDown[mouseButtonCode] = value;
	}
	else
	{
		// Current
		Input::mouseButtonDown.insert(
			std::pair<Mouse, bool>(
				mouseButtonCode,
				value
			)
		);

		// Last
		Input::lastmouseButtonDown.insert(
			std::pair<Mouse, bool>(
				mouseButtonCode,
				!value
			)
		);
	}
}

void Input::setCursorPosition(const int& newCursorX, const int& newCursorY)
{
	Input::shouldSetMousePos = true;
	Input::requestedMouseX = newCursorX;
	Input::requestedMouseY = newCursorY;
}

void Input::setHideCursor(const bool& hide)
{
	Input::shouldHideCursor = hide;
}

const glm::vec2 Input::getMouseUITranslated()
{
	return ResTranslator::toInternalPos(glm::vec2((float)Input::getMouseX(), (float)Input::getMouseY()));
}
