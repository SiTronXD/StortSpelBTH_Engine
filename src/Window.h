#pragma once

#include <string>
#include <vector>
#include "imgui_impl_sdl.h"     // Need to be included in header

namespace vk
{
    class Instance;
    class SurfaceKHR;
};

struct SDL_Window;
union SDL_Event;

class Window
{
private:
    bool isRunning;

public:
    SDL_Window* windowHandle;

    Window();
    Window(Window const& ref) = delete;
    Window(Window&& ref) noexcept = delete;
    Window& operator=(Window const& ref) = delete;
    Window& operator=(Window&&ref) noexcept   = delete;
    ~Window();

    void initWindow(const std::string &name, int width, int height);
    void registerResizeEvent(bool& listener);
    void update();

    void initImgui();
    void shutdownImgui();

    void createVulkanSurface(const vk::Instance& instance, vk::SurfaceKHR& outputSurface);
    void getVulkanExtensions(std::vector<const char*>& outputExtensions);
    void getSize(int& outputWidth, int& outputHeight);

    const bool& getIsRunning() const { return this->isRunning; }
};

////__attribute__((unused))
[[maybe_unused]]  
int resizingEventWatcher(void *data, SDL_Event *event);