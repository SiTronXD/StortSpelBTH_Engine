#pragma once

#include <map>
#include <SDL2/SDL.h>

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

	HOME = SDLK_HOME
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

	static std::map<Keys, bool> keyDown;
	static std::map<Keys, bool> lastKeyDown;

	static std::map<Mouse, bool> mouseButtonDown;
	static std::map<Mouse, bool> lastmouseButtonDown;

	static int cursorX;
	static int cursorY;
	static int lastCursorX;
	static int lastCursorY;

	static void setCursor(const int& newCursorX, const int& newCursorY);
	static void update();

	static void setKey(const Keys& keyCode, const bool& value);
	static void setMouseButton(const Mouse& mouseButtonCode, const bool& value);

public:
	static inline bool isKeyDown(const Keys& key) { return Input::keyDown[key]; }
	static inline bool isKeyPressed(const Keys& key) { return Input::keyDown[key] && !Input::lastKeyDown[key]; }
	static inline bool isMouseButtonDown(const Mouse& mouse) { return Input::mouseButtonDown[mouse]; }
	static inline bool isMouseButtonPressed(const Mouse& mouse) { return Input::mouseButtonDown[mouse] && !Input::lastmouseButtonDown[mouse]; }
	static inline const int& getMouseX() { return Input::cursorX; }
	static inline const int& getMouseY() { return Input::cursorY; }
	static inline const int getMouseDeltaX() { return Input::lastCursorX - Input::cursorX; }
	static inline const int getMouseDeltaY() { return Input::lastCursorY - Input::cursorY; }
};