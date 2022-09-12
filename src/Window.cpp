#include "Window.hpp"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>

#include "Window.hpp"
#include "tracy/Tracy.hpp"
#include "Input.hpp"
#include "Log.hpp"
#include "VulkanInstance.hpp"

Window::Window()
    : windowHandle(nullptr),
    isRunning(true)
{}

void Window::initWindow(
    const std::string &name, 
    const int width, 
    const int height)
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
    Input::update();

    // Update current keys
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
                (Keys) event.key.keysym.sym,
                event.type == SDL_KEYDOWN
            );
        }

        // Register mouse button press/release
        if (event.type == SDL_MOUSEBUTTONDOWN || 
            event.type == SDL_MOUSEBUTTONUP)
        {
            Input::setMouseButton(
                (Mouse) event.button.button,
                event.type == SDL_MOUSEBUTTONDOWN
            );
        }
    }

    // Update mouse position
    int mouseX = 0;
    int mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);
    Input::setCursor(mouseX, mouseY);
}

void Window::initImgui()
{
    ImGui_ImplSDL2_InitForVulkan(this->windowHandle);
}

void Window::shutdownImgui()
{
    ImGui_ImplSDL2_Shutdown();
}

void Window::createVulkanSurface(
    VulkanInstance& instance,
    vk::SurfaceKHR& outputSurface)
{
    VkSurfaceKHR sdlSurface{};
    SDL_bool result = SDL_Vulkan_CreateSurface(
        (this->windowHandle),
        instance.getVkInstance(),
        &sdlSurface
    );

    if (result != SDL_TRUE) 
    {
        Log::error("Failed to create (SDL) surface.");
    }

    outputSurface = sdlSurface;
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

void Window::getSize(int& outputWidth, int& outputHeight)
{
    SDL_GetWindowSize(this->windowHandle, &outputWidth, &outputHeight);
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