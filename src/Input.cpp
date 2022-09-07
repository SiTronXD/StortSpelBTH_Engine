#include "Input.h"
// #include "../Dev/Log.h"

std::map<Keys, bool> Input::keyDown;
std::map<Keys, bool> Input::lastKeyDown;
// bool Input::mouseButtonDown[MAX_NUM_MOUSE_BUTTONS]{};

/*float Input::cursorX = 0.0f;
float Input::cursorY = 0.0f;
float Input::lastCursorX = 0.0f;
float Input::lastCursorY = 0.0f;*/

/*void Input::setCursor(const float& newCursorX, const float& newCursorY)
{
	// Update last position
	Input::lastCursorX = Input::cursorX;
	Input::lastCursorY = Input::cursorY;

	// Update current position
	Input::cursorX = newCursorX;
	Input::cursorY = newCursorY;
}*/

void Input::updateLastKeys()
{
	for (auto const& [key, val] : Input::keyDown)
	{
		Input::lastKeyDown[key] = val;
	}
}

void Input::setKey(const SDL_Keycode& keyCode, const bool& value)
{
	// Set value
	if (Input::keyDown.count((Keys)keyCode))
	{
		Input::keyDown[(Keys)keyCode] = value;
	}
	// Insert new element
	else
	{
		Input::keyDown.insert(
			std::pair<Keys, bool>(
				(Keys)keyCode,
				value
			)
		);

		Input::lastKeyDown.insert(
			std::pair<Keys, bool>(
				(Keys)keyCode,
				!value
			)
		);
	}
}
