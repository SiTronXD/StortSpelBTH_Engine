#include "pch.h"
#include "Window.hpp"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>

#include "Window.hpp"
#include "tracy/Tracy.hpp"
#include "Input.hpp"
#include "../dev/Log.hpp"
#include "../graphics/vulkan/VulkanInstance.hpp"
#include "../resource_management/Configurator.hpp"

using namespace vengine_helper::config;

Window::Window()
    : windowHandle(nullptr),
    isRunning(true),
    width(0),
    height(0)
{}

void Window::initWindow(
    const std::string &name)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    this->titleName = name;

    // Initializes SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Get size from config
    this->width = DEF<int>(W_WIDTH);
    this->height = DEF<int>(W_HEIGHT);

    // Configure SDL Window...    
    this->windowHandle = SDL_CreateWindow(
        this->titleName.c_str(),
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        this->width, 
        this->height, 
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
    // Hide cursor if it should
    if (Input::shouldHideCursor != Input::lastShouldHideCursor)
    {
        SDL_ShowCursor(
            Input::shouldHideCursor ? 
            SDL_DISABLE :
            SDL_ENABLE
        );

        // Relative mode for mouse delta movement
        SDL_SetRelativeMouseMode(
            Input::shouldHideCursor ? 
            SDL_TRUE : 
            SDL_FALSE
        );
    }

    // Update input
    Input::update();

    // Update current keys
    SDL_Event event;
    Input::setDeltaCursor(0, 0);
    Input::setDeltaScrollWheel(0);
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

        // Mouse motion
        if (event.type == SDL_MOUSEMOTION)
        {
            Input::setDeltaCursor(
                event.motion.xrel, 
                event.motion.yrel
            );
        }

        // Scroll wheel
        if (event.type == SDL_MOUSEWHEEL)
        {
            Input::setDeltaScrollWheel(event.wheel.y);
        }
    }

    // Set mouse pos
    if (Input::shouldSetMousePos)
    {
        Input::shouldSetMousePos = false;

        // Set mouse position
        SDL_WarpMouseInWindow(
            this->windowHandle,
            Input::requestedMouseX,
            Input::requestedMouseY
        );
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

void Window::setFullscreen(bool fullscreen)
{
    // Get size from config
    this->width = DEF<int>(W_WIDTH);
    this->height = DEF<int>(W_HEIGHT);

    // Use display size when fullscreen
    if (fullscreen)
    {
        SDL_DisplayMode dm{};
        SDL_GetCurrentDisplayMode(0, &dm);
        this->width = dm.w;
        this->height = dm.h;
    }

    // Apply new(__FILE__, __LINE__) size
    SDL_SetWindowSize(
        this->windowHandle, 
        this->width, 
        this->height
    );

    // Fullscreen
    SDL_SetWindowFullscreen(
        this->windowHandle,
        fullscreen ? SDL_WINDOW_FULLSCREEN : 0
    );
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
    unsigned int sdlExtensionCount = 0;        //  may require multiple extension  
    SDL_Vulkan_GetInstanceExtensions(
        this->windowHandle, 
        &sdlExtensionCount, 
        nullptr
    );

    // Store the extensions in sdlExtensions, and the number of extensions in sdlExtensionCount
    outputExtensions.resize(sdlExtensionCount);

    //  Get SDL Extensions
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

void Window::close()
{
    this->isRunning = false;
}

Window::~Window() {

    SDL_DestroyWindow(this->windowHandle);
}

// /__attribute__((unused))
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