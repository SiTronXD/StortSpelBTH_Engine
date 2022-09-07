#pragma once

#include <string>
#include "imgui_impl_sdl.h"     // Need to be included in header

struct SDL_Window;
union SDL_Event;

class Window
{
private:
    bool isRunning;

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

    const bool& getIsRunning() const { return this->isRunning; }
};

////__attribute__((unused))
[[maybe_unused]]  
int resizingEventWatcher(void *data, SDL_Event *event);