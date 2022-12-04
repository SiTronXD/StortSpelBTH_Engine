#pragma once 
 #include "op_overload.hpp"

#include <string>
#include <vector>
#include "imgui_impl_sdl.h"     // Need to be included in header

class VulkanInstance;

namespace vk
{
    class SurfaceKHR;
};

struct SDL_Window;
union SDL_Event;

class Window
{
private:
    bool isRunning;

    std::string titleName;

    SDL_Window* windowHandle;
    int width;
    int height;

public:
    Window();
    Window(Window const& ref) = delete;
    Window(Window&& ref) noexcept = delete;
    Window& operator=(Window const& ref) = delete;
    Window& operator=(Window&&ref) noexcept   = delete;
    ~Window();

    void initWindow(const std::string &name);
    void registerResizeEvent(bool& listener);
    void update();

    void initImgui();
    void shutdownImgui();
    void setFullscreen(bool fullscreen);

    void createVulkanSurface(VulkanInstance& instance, vk::SurfaceKHR& outputSurface);
    void getVulkanExtensions(std::vector<const char*>& outputExtensions);
    void getSize(int& outputWidth, int& outputHeight);

    void close();
    const bool& getIsRunning() const { return this->isRunning; }

    inline SDL_Window*& getWindowHandle() { return this->windowHandle; }
    inline const std::string& getTitleName() { return this->titleName; }
};

////__attribute__((unused))
[[maybe_unused]]  
int resizingEventWatcher(void *data, SDL_Event *event);