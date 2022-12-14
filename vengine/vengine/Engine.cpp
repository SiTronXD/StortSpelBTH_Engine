#include "pch.h"
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
    : networkHandler(nullptr)
{
}

Engine::~Engine()
{
    delete this->networkHandler;
}

void Engine::setCustomNetworkHandler(NetworkHandler* networkHandler)
{
    this->networkHandler = networkHandler;
}

void Engine::run(std::string appName, std::string startScenePath, Scene* startScene)
{
    using namespace vengine_helper::config;
    loadConfIntoMemory(); // load config data into memory

    //Filter Log Output 
    Log::addFilter("bt");

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

    // No custom network handler was set
    if (this->networkHandler == nullptr) { this->networkHandler = new NetworkHandler(); }

    //  Set references to other systems
    this->sceneHandler.setNetworkHandler(networkHandler);
    this->sceneHandler.setScriptHandler(&scriptHandler);
    this->sceneHandler.setResourceManager(&resourceManager);
    this->sceneHandler.setPhysicsEngine(&physicsEngine);
    this->sceneHandler.setVulkanRenderer(&renderer);
    this->sceneHandler.setUIRenderer(&uiRenderer);
    this->sceneHandler.setDebugRenderer(&debugRenderer);
    this->sceneHandler.setAIHandler(&aiHandler);
    this->sceneHandler.setWindow(&window);
    this->sceneHandler.setAudioHandler(&audioHandler);
    this->networkHandler->setSceneHandler(&sceneHandler);
	this->networkHandler->setResourceManager(&resourceManager);
	this->physicsEngine.setSceneHandler(&sceneHandler);
    this->scriptHandler.setSceneHandler(&sceneHandler);
    this->scriptHandler.setResourceManager(&resourceManager);
    this->scriptHandler.setPhysicsEngine(&physicsEngine);
    this->scriptHandler.setUIRenderer(&uiRenderer);
    this->scriptHandler.setDebugRenderer(&debugRenderer);
    this->uiRenderer.setSceneHandler(&sceneHandler);
    this->debugRenderer.setSceneHandler(&sceneHandler);
    this->scriptHandler.setNetworkHandler(networkHandler);
    this->audioHandler.setSceneHandler(&sceneHandler);

    // Initialize the start scene
    if (startScene == nullptr) { startScene = new Scene(); }
    this->sceneHandler.setScene(startScene, startScenePath);
    this->sceneHandler.updateToNextScene();

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
        this->sceneHandler.update();
        this->scriptHandler.update();
        this->audioHandler.update();
		this->networkHandler->update();

#if defined(_CONSOLE) || defined(_STATISTICS) // Debug/Release, but not distribution        
        static bool debugInfo = true;

        // Reset avg fps
        if (Input::isKeyPressed(Keys::P))
        {
            this->avgFPS = 0.0f;
            this->elapsedFrames = 0u;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        ImGui::SetWindowPos(ImVec2{0.f,0.f}, ImGuiCond_Once);
        ImGui::Begin("Debug info",&debugInfo,ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize );
            float t = 1.0f / float(this->elapsedFrames + 1u);
            this->avgFPS = (1.0f - t) * this->avgFPS + t * io.Framerate;
            this->elapsedFrames++;
            ImGui::Text("FPS: %.3f ms/f (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("AVG FPS: %.3f ms/f (%.1f FPS) (%u elapsedFrames)", 1000.0f / this->avgFPS, this->avgFPS, this->elapsedFrames);
#if defined(_WIN32)
            ImGui::Text("RAM: %.3f MB", this->statsCollector.getRamUsage());
            ImGui::Text("VRAM: %.3f MB", this->statsCollector.getVramUsage());
#endif
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
    this->networkHandler->deleteServer();
    this->scriptHandler.cleanup();
    this->aiHandler.clean();
    this->resourceManager.cleanUp();

    renderer.cleanup();
}
