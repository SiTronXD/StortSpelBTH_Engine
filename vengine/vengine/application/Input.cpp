#include "Input.hpp"
// #include "../Dev/Log.hpp"

std::map<Keys, bool> Input::keyDown;
std::map<Keys, bool> Input::lastKeyDown;
std::map<Mouse, bool> Input::mouseButtonDown;
std::map<Mouse, bool> Input::lastmouseButtonDown;

int Input::cursorX = 0.0;
int Input::cursorY = 0.0;
int Input::lastCursorX = 0.0;
int Input::lastCursorY = 0.0;
bool Input::isCursorLocked = false;
bool Input::shouldHideCursor = false;
bool Input::lastShouldHideCursor = false;

void Input::setCursor(const int& newCursorX, const int& newCursorY)
{
	// Update last position
	Input::lastCursorX = Input::cursorX;
	Input::lastCursorY = Input::cursorY;

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

void Input::setLockCursorPosition(const bool& lock)
{
	Input::isCursorLocked = lock;
}

void Input::setHideCursor(const bool& hide)
{
	Input::shouldHideCursor = hide;
}