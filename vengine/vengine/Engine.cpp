#include "Engine.hpp"

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

// /We need to take care of ALL error messages... vulkan does not report by default...
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <vector>

#include "application/Window.hpp"
#include "application/Input.hpp"
#include "application/Time.hpp"
#include "graphics/VulkanRenderer.hpp"
#include "resource_management/Configurator.hpp"

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

void Engine::run(std::string appName, std::string startScenePath, Scene* startScene)
{
    using namespace vengine_helper::config;
    loadConfIntoMemory(); // load config data into memory

    // Window
    Window window;
    window.initWindow(appName);
    window.setFullscreen(DEF<bool>(W_FULLSCREEN));
    
    // Create vulkan renderer instance
    VulkanRenderer renderer;
    if (renderer.init(
        &window, 
        std::move(appName), 
        &this->resourceManager,
        &this->uiRenderer,
        &this->debugRenderer) == 1)
    {
        std::cout << "EXIT_FAILURE" << std::endl;
    }
    window.registerResizeEvent(renderer.getWindowResized());

    // Set references to other systems
    this->sceneHandler.setNetworkHandler(&networkHandler);
    this->sceneHandler.setScriptHandler(&scriptHandler);
    this->sceneHandler.setResourceManager(&resourceManager);
    this->sceneHandler.setPhysicsEngine(&physicsEngine);
    this->sceneHandler.setVulkanRenderer(&renderer);
    this->sceneHandler.setUIRenderer(&uiRenderer);
    this->sceneHandler.setDebugRenderer(&debugRenderer);
    this->networkHandler.setSceneHandler(&sceneHandler);
	this->physicsEngine.setSceneHandler(&sceneHandler);
    this->scriptHandler.setSceneHandler(&sceneHandler);
    this->scriptHandler.setResourceManager(&resourceManager);
    this->debugRenderer.setSceneHandler(&sceneHandler);
    this->scriptHandler.setNetworkHandler(&networkHandler);

    // Initialize the start scene
    if (startScene == nullptr) { startScene = new Scene(); }
    this->sceneHandler.setScene(startScene, startScenePath);
    this->sceneHandler.updateToNextScene();

    // Temporary, should be called before creating the scene
    this->audioHandler.setSceneHandler(&sceneHandler);

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
        this->scriptHandler.update();
		    this->physicsEngine.update();
		    this->networkHandler.updateNetwork();
        this->audioHandler.update();
        this->sceneHandler.update();

        static bool open = true;
        ImGui::ShowDemoWindow(&open);

        ImGui::Begin("Another Window", &open);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            open = false;
        ImGui::End();
        
        // TODO: remove this
        static bool lockCursor = false;
        if (Input::isKeyPressed(Keys::ESC))
        {
            lockCursor = !lockCursor;
            Input::setLockCursorPosition(lockCursor);
            Input::setHideCursor(lockCursor);
        }

        // ------------------------------------
        this->sceneHandler.updateToNextScene();

        renderer.draw(this->sceneHandler.getScene());

#ifndef VENGINE_NO_PROFILING
        FrameMark;
#endif
    }
    this->networkHandler.deleteServer();
    this->scriptHandler.cleanup();

    renderer.cleanup();
}
