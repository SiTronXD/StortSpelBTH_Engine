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

    // Get Imgui IO, used by fps counter
    ImGuiIO&  io = ImGui::GetIO();

    //  Set references to other systems
    this->sceneHandler.setNetworkHandler(&networkHandler);
    this->sceneHandler.setScriptHandler(&scriptHandler);
    this->sceneHandler.setResourceManager(&resourceManager);
    this->sceneHandler.setPhysicsEngine(&physicsEngine);
    this->sceneHandler.setVulkanRenderer(&renderer);
    this->sceneHandler.setUIRenderer(&uiRenderer);
    this->sceneHandler.setDebugRenderer(&debugRenderer);
    this->sceneHandler.setAIHandler(&aiHandler);
    this->sceneHandler.setWindow(&window);
    this->sceneHandler.setAudioHandler(&audioHandler);
    this->networkHandler.setSceneHandler(&sceneHandler);
	this->physicsEngine.setSceneHandler(&sceneHandler);
    this->scriptHandler.setSceneHandler(&sceneHandler);
    this->scriptHandler.setResourceManager(&resourceManager);
    this->scriptHandler.setPhysicsEngine(&physicsEngine);
    this->scriptHandler.setUIRenderer(&uiRenderer);
    this->scriptHandler.setDebugRenderer(&debugRenderer);
    this->debugRenderer.setSceneHandler(&sceneHandler);
    this->scriptHandler.setNetworkHandler(&networkHandler);
    this->audioHandler.setSceneHandler(&sceneHandler);

    // Initialize the start scene
    if (startScene == nullptr) { startScene = new Scene(); }
    this->sceneHandler.setScene(startScene, startScenePath);
    this->sceneHandler.updateToNextScene();

    //Needs to run after scene is initialized
    this->aiHandler.init(&this->sceneHandler);    

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
		this->physicsEngine.update(Time::getDT());
        this->aiHandler.update();
        this->sceneHandler.update();
        this->scriptHandler.update();
        this->audioHandler.update();
		this->networkHandler.updateNetwork();                


#if defined(_CONSOLE) // Debug/Release, but not distribution        
        static bool debugInfo = true;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        ImGui::SetWindowPos(ImVec2{0.f,0.f}, ImGuiCond_Once);
        ImGui::Begin("Debug info",&debugInfo,ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize );
            ImGui::Text("FPS: avg. %.3f ms/f (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);                                    
        ImGui::End();
        ImGui::PopStyleVar();
#endif                
        
        //  ------------------------------------
        this->sceneHandler.updateToNextScene();

        this->sceneHandler.prepareForRendering();
        renderer.draw(this->sceneHandler.getScene());

#ifndef VENGINE_NO_PROFILING
        FrameMark;
#endif
    }
    this->networkHandler.deleteServer();
    this->scriptHandler.cleanup();
    this->aiHandler.clean();

    renderer.cleanup();
}
