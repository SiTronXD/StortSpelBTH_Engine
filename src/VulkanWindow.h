#pragma once

#include <string>
struct SDL_Window;
union SDL_Event;
class VulkanWindow{
private:

public:
    SDL_Window * sdl_window;

    VulkanWindow();
    VulkanWindow(VulkanWindow const &ref)       = delete;
    VulkanWindow(VulkanWindow &&ref) noexcept   = delete;
    VulkanWindow& operator=(VulkanWindow const &ref)       = delete;
    VulkanWindow& operator=(VulkanWindow &&ref) noexcept   = delete;
    ~VulkanWindow();

    void initWindow(const std::string &name, int width, int height);
    void registerResizeEvent(bool& listener);

};

////__attribute__((unused))
[[maybe_unused]]  
int resizingEventWatcher(void *data, SDL_Event *event);