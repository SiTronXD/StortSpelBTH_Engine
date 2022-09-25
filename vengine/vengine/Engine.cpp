#include "Engine.hpp"

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

///We need to take care of ALL error messages... vulkan does not report by default...
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <vector>

#include "application/Window.hpp"
#include "application/Input.hpp"
#include "application/Time.hpp"
#include "graphics/VulkanRenderer.hpp"
#include "loaders/Configurator.hpp"

#include <chrono>
#include <functional>

#include "imgui_impl_vulkan.h"
#include "backends/imgui_impl_vulkan.h"

Engine::Engine()
{
	
}

Engine::~Engine()
{
}

void Engine::run(Scene* startScene)
{
    this->sceneHandler.setScene(startScene);
    this->sceneHandler.updateToNextScene();
    this->sceneHandler.setNetworkHandler(&networkHandler);
    this->networkHandler.setSceneHandler(&sceneHandler);

    using namespace vengine_helper::config;
    loadConfIntoMemory(); // load config data into memory

    Window window;
    window.initWindow(
        "Some Program", 
        DEF<int>(W_WIDTH), 
        DEF<int>(W_HEIGHT)
    );
    
    // Creating Vulkan Renderer Instance
    auto renderer = VulkanRenderer();
    if (renderer.init(&window, "Some Program", &this->resourceMan) == 1)
    {
        std::cout << "EXIT_FAILURE" << std::endl;
    }

    window.registerResizeEvent(renderer.getWindowResized());

    renderer.initMeshes(this->sceneHandler.getScene());

    // Game loop
    while (window.getIsRunning())
    {
        window.update();

        if (Input::isKeyPressed(Keys::HOME))
        {
            std::cout << "Home was pressed! generating vma dump" << "\n";
            renderer.generateVmaDump();
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window.getWindowHandle());
        ImGui::NewFrame();

        Time::updateDeltaTime();
        this->sceneHandler.update();
        this->networkHandler.updateNetwork();

        static bool open = true;
        ImGui::ShowDemoWindow(&open);

        ImGui::Begin("Another Window", &open);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            open = false;
        ImGui::End();
        
        // ------------------------------------
        this->sceneHandler.updateToNextScene();

        renderer.draw(this->sceneHandler.getScene());

#ifndef VENGINE_NO_PROFILING
        FrameMark;
#endif
    }
    this->networkHandler.deleteServer();

    renderer.cleanup();
}
