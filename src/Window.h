#pragma once

#include <string>

struct SDL_Window;
union SDL_Event;

class Window
{
private:

public:
    SDL_Window * sdl_window;

    Window();
    Window(Window const& ref) = delete;
    Window(Window&& ref) noexcept = delete;
    Window& operator=(Window const& ref) = delete;
    Window& operator=(Window&&ref) noexcept   = delete;
    ~Window();

    void initWindow(const std::string &name, int width, int height);
    void registerResizeEvent(bool& listener);
    void update();
};

////__attribute__((unused))
[[maybe_unused]]  
int resizingEventWatcher(void *data, SDL_Event *event);