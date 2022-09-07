#include "Window.h"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>
#include "tracy/Tracy.hpp"
#include "Input.h"

Window::Window()
    : windowHandle(nullptr),
    isRunning(true)
{}

void Window::initWindow(const std::string &name, const int width, const int height)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    ///Initializes sdl window
    SDL_Init(SDL_INIT_VIDEO);

    ///Configure SDL Window...    
    this->windowHandle = SDL_CreateWindow(
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

    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT ||
            (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(this->windowHandle)))
        {
            this->isRunning = false;
        }

        // Register key press/release
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        {
            Input::setKey(
                event.key.keysym.sym,
                event.type == SDL_KEYDOWN
            );
        }
    }
}

void Window::getVulkanExtensions(
    std::vector<const char*>& outputExtensions)
{
    unsigned int sdlExtensionCount = 0;        /// may require multiple extension  
    SDL_Vulkan_GetInstanceExtensions(
        this->windowHandle, 
        &sdlExtensionCount, 
        nullptr
    );

    ///Store the extensions in sdlExtensions, and the number of extensions in sdlExtensionCount
    outputExtensions.resize(sdlExtensionCount);

    /// Get SDL Extensions
    SDL_Vulkan_GetInstanceExtensions(
        this->windowHandle, 
        &sdlExtensionCount, 
        outputExtensions.data()
    );
}

Window::~Window() {

    SDL_DestroyWindow(this->windowHandle);
}

////__attribute__((unused))
[[maybe_unused]]
int resizingEventWatcher(void* data, SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT &&
		event->window.event == SDL_WINDOWEVENT_RESIZED)
	{
		*(reinterpret_cast<bool*>(data)) = true;
	}

	return 0;
}