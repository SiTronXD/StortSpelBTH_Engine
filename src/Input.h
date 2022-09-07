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

/*enum class Mouse
{
	LEFT_BUTTON = GLFW_MOUSE_BUTTON_LEFT,
	RIGHT_BUTTON = GLFW_MOUSE_BUTTON_RIGHT
};*/

class Input
{
private:
	friend class Window;

	static std::map<Keys, bool> keyDown;
	static std::map<Keys, bool> lastKeyDown;
	// static bool mouseButtonDown[MAX_NUM_MOUSE_BUTTONS];

	/*static float cursorX;
	static float cursorY;
	static float lastCursorX;
	static float lastCursorY;*/

	// static void setCursor(const float& newCursorX, const float& newCursorY);
	static void updateLastKeys();

	static void setKey(const SDL_Keycode& keyCode, const bool& value);

public:
	static inline bool isKeyDown(const Keys& key) { return Input::keyDown[key]; }
	static inline bool isKeyPressed(const Keys& key) { return Input::keyDown[key] && !Input::lastKeyDown[key]; }
	// static inline bool isMouseButtonDown(const Mouse& mouse) { return mouseButtonDown[(int) mouse]; }
	// static inline const float getMouseDeltaX() { return Input::lastCursorX - Input::cursorX; }
	// static inline const float getMouseDeltaY() { return Input::lastCursorY - Input::cursorY; }
};