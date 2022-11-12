#pragma once

#include <map>
#include <SDL2/SDL.h>
#include <string>
#include <glm/glm.hpp>

enum class Keys
{
	A = SDLK_a,
	B = SDLK_b,
	C = SDLK_c,
	D = SDLK_d,
	E = SDLK_e,
	F = SDLK_f,
	G = SDLK_g,
	H = SDLK_h,
	I = SDLK_i,
	J = SDLK_j,
	K = SDLK_k,
	L = SDLK_l,
	M = SDLK_m,
	N = SDLK_n,
	O = SDLK_o,
	P = SDLK_p,
	Q = SDLK_q,
	R = SDLK_r,
	S = SDLK_s,
	T = SDLK_t,
	U = SDLK_u,
	V = SDLK_v,
	W = SDLK_w,
	X = SDLK_x,
	Y = SDLK_y,
	Z = SDLK_z,

	LEFT = SDLK_LEFT,
	RIGHT = SDLK_RIGHT,
	UP = SDLK_UP,
	DOWN = SDLK_DOWN,

	DOT = SDLK_PERIOD,
	BACK = SDLK_BACKSPACE,

	ONE = SDLK_1,
	TWO = SDLK_2,
	THREE = SDLK_3,
	FOUR = SDLK_4,
	FIVE = SDLK_5,
	SIX = SDLK_6,
	SEVEN = SDLK_7,
	EIGHT = SDLK_8,
	NINE = SDLK_9,
	ZERO = SDLK_0,

	HOME = SDLK_HOME,
	SHIFT = SDLK_LSHIFT,
	CTRL = SDLK_LCTRL,
	SPACE = SDLK_SPACE,
	ESC = SDLK_ESCAPE
};

enum class Mouse
{
	LEFT = SDL_BUTTON_LEFT,
	MIDDLE = SDL_BUTTON_MIDDLE,
	RIGHT = SDL_BUTTON_RIGHT
};

class Input
{
private:
	friend class Window;
	friend class InputLua;

    inline static const std::map<std::string, int> keysMap {
		{ "A" , SDLK_a },
		{ "B" , SDLK_b },
		{ "C" , SDLK_c },
		{ "D" , SDLK_d },
		{ "E" , SDLK_e },
		{ "F" , SDLK_f },
		{ "G" , SDLK_g },
		{ "H" , SDLK_h },
		{ "I" , SDLK_i },
		{ "J" , SDLK_j },
		{ "K" , SDLK_k },
		{ "L" , SDLK_l },
		{ "M" , SDLK_m },
		{ "N" , SDLK_n },
		{ "O" , SDLK_o },
		{ "P" , SDLK_p },
		{ "Q" , SDLK_q },
		{ "R" , SDLK_r },
		{ "S" , SDLK_s },
		{ "T" , SDLK_t },
		{ "U" , SDLK_u },
		{ "V" , SDLK_v },
		{ "W" , SDLK_w },
		{ "X" , SDLK_x },
		{ "Y" , SDLK_y },
		{ "Z" , SDLK_z },

		{ "LEFT" , SDLK_LEFT },
		{ "RIGHT" , SDLK_RIGHT },
		{ "UP" , SDLK_UP },
		{ "DOWN" , SDLK_DOWN },

		{ "DOT", SDLK_PERIOD }, 
		{ "BACK", SDLK_BACKSPACE },
		
		{ "ONE" , SDLK_1 },
		{ "TWO" , SDLK_2 },
		{ "THREE" , SDLK_3 },
		{ "FOUR" , SDLK_4 },
		{ "FIVE" , SDLK_5 },
		{ "SIX" , SDLK_6 },
		{ "SEVEN" , SDLK_7 },
		{ "EIGHT" , SDLK_8 },
		{ "NINE" , SDLK_9 },
		{ "ZERO" , SDLK_0 },

		{ "HOME" , SDLK_HOME },
		{ "SHIFT" , SDLK_LSHIFT },
		{ "CTRL" , SDLK_LCTRL },
		{ "SPACE" , SDLK_SPACE },
		{ "ESCAPE", SDLK_ESCAPE }, 
		{}
    };
    inline static const std::map<std::string, int> mouseMap {
        { "LEFT", SDL_BUTTON_LEFT },
        { "MIDDLE", SDL_BUTTON_MIDDLE },
        { "RIGHT", SDL_BUTTON_RIGHT }
    };

	static std::map<Keys, bool> keyDown;
	static std::map<Keys, bool> lastKeyDown;

	static std::map<Mouse, bool> mouseButtonDown;
	static std::map<Mouse, bool> lastmouseButtonDown;

	static int cursorX;
	static int cursorY;
	static int deltaCursorX;
	static int deltaCursorY;
	static int requestedMouseX;
	static int requestedMouseY;

	static int deltaScrollWheel;

	static bool shouldSetMousePos;
	static bool shouldHideCursor;
	static bool lastShouldHideCursor;

	static void setDeltaCursor(const int& deltaCursorX, const int& deltaCursorY);
	static void setDeltaScrollWheel(const int& deltaScrollWheel);
	static void setCursor(const int& newCursorX, const int& newCursorY);
	static void update();

	static void setKey(const Keys& keyCode, const bool& value);
	static void setMouseButton(const Mouse& mouseButtonCode, const bool& value);

public:
	static void setCursorPosition(const int& newCursorX, const int& newCursorY);
	static void setHideCursor(const bool& hide);

	static inline bool isKeyDown(const Keys& key) { return Input::keyDown[key]; }
	static inline bool isKeyPressed(const Keys& key) { return Input::keyDown[key] && !Input::lastKeyDown[key]; }
	static inline bool isKeyReleased(const Keys& key) { return !Input::keyDown[key] && Input::lastKeyDown[key]; }
	static inline bool isMouseButtonDown(const Mouse& mouse) { return Input::mouseButtonDown[mouse]; }
	static inline bool isMouseButtonPressed(const Mouse& mouse) { return Input::mouseButtonDown[mouse] && !Input::lastmouseButtonDown[mouse]; }
	static inline bool isMouseButtonReleased(const Mouse& mouse) { return !Input::mouseButtonDown[mouse] && Input::lastmouseButtonDown[mouse]; }
	static inline const int& getMouseX() { return Input::cursorX; }
	static inline const int& getMouseY() { return Input::cursorY; }
	static inline const int getMouseDeltaX() { return Input::deltaCursorX; }
	static inline const int getMouseDeltaY() { return Input::deltaCursorY; }
	static inline const int getScrollWheelDelta() { return Input::deltaScrollWheel; }
	static const glm::vec2 getMouseUITranslated();
};