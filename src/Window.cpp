#include "Window.h"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>
#include "tracy/Tracy.hpp"
#include "Input.h"

Window::Window()
    : sdl_window(nullptr)
{}

void Window::initWindow(const std::string &name, const int width, const int height)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    ///Initializes sdl window
    SDL_Init(SDL_INIT_VIDEO);

    ///Configure SDL Window...    
    this->sdl_window = SDL_CreateWindow(
        name.c_str(), 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        width, 
        height, 
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
}

void Window::registerResizeEvent(bool& listener)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    

    SDL_AddEventWatch(resizingEventWatcher, &listener);
}

void Window::update()
{
    Input::updateLastKeys();

    /*while (SDL_PollEvent(&event) != 0) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT ||
            (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(this->window.sdl_window)))
        {
            quitting = true;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_HOME:
                std::cout << "Home was pressed! generating vma dump" << "\n";
                this->generateVmaDump();

            default:;
            }
        }
    }

    for (auto&& event : events) 
    {
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_b:
                std::cout << "Testing thing in loop" << "\n";
            default:;
            }
        }
    }*/
}

Window::~Window() {

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