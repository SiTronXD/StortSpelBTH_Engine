#include "VulkanWindow.h"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "tracy/Tracy.hpp"

VulkanWindow::VulkanWindow()
:   sdl_window(nullptr)//glfw_window(nullptr)
{}

void VulkanWindow::initWindow(const std::string &name, const int width, const int height)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    
    ///Initializes sdl window
    SDL_Init(SDL_INIT_VIDEO);
    ///Configure SDL Window...    
    this->sdl_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
}
void VulkanWindow::registerResizeEvent(bool& listener)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    

    SDL_AddEventWatch(resizingEventWatcher, &listener);
}

VulkanWindow::~VulkanWindow() {

    //glfwDestroyWindow(glfw_window);
    SDL_DestroyWindow(sdl_window);
}
////__attribute__((unused))
[[maybe_unused]]  
int resizingEventWatcher(void *data, SDL_Event *event) {
  if (event->type == SDL_WINDOWEVENT &&
      event->window.event == SDL_WINDOWEVENT_RESIZED) {
    *(reinterpret_cast<bool *>(data)) = true;
  }
  return 0;
}