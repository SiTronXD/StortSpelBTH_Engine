#include "Engine.hpp"

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

///We need to take care of ALL error messages... vulkan does not report by default...
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <vector>

#include "Window.hpp"
#include "Input.hpp"
#include "Time.hpp"
#include "VulkanRenderer.hpp"
#include "Configurator.hpp"

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
    loadConfIntoMemory(); /// load config data into memory

    Window window;
    window.initWindow(
        "Some Program", 
        DEF<int>(W_WIDTH), 
        DEF<int>(W_HEIGHT)
    );

    /// Creating Vulkan Renderer Instance
    auto renderer = VulkanRenderer();
    if (renderer.init(&window, "Some Program") == 1)
    {
        std::cout << "EXIT_FAILURE" << std::endl;
    }

    window.registerResizeEvent(renderer.getWindowResized());

    renderer.initMeshes(this->sceneHandler.getScene());

    double angle = 0.F;

    // int ghostModelIndex = renderer.createModel("ghost.obj");

    glm::mat4 correctSize = glm::mat4(1.F);
    float scale = 0.2F;
    correctSize = glm::scale(correctSize, glm::vec3(scale, scale, scale));

    glm::mat4 correctRot = glm::mat4(1.F);

    scale = 5.F;

    /*myRenderer.updateModel(ghostModelIndex, correctRot);
    correctRot = glm::translate(glm::mat4(1.F), glm::vec3(0.F, 10.F, -100.F));
    correctRot = glm::rotate(correctRot, glm::radians(-90.F), glm::vec3(1.F, 0.F, 0.F));
    correctRot = glm::rotate(correctRot, glm::radians(-90.F), glm::vec3(0.F, 0.F, 1.F));
    correctRot = glm::scale(correctRot, glm::vec3(scale, scale, scale));
    myRenderer.updateModel(ghostModelIndex, correctRot);*/

    // Game loop
    while (window.getIsRunning())
    {
        window.update();

        if (Input::isKeyPressed(Keys::HOME))
        {
            std::cout << "Home was pressed! generating vma dump" << "\n";
            renderer.generateVmaDump();
        }

        //SDL_PollEvent(&event);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window.windowHandle);
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

        angle += 10.0 * Time::getDT();
        angle = angle > 360.0 ? 0.0 : angle;

        glm::mat<4, 4, double> ghost_model_matrix = correctRot;
        glm::mat<4, 4, double> sponza_model_matrix = correctSize;

        ghost_model_matrix = glm::rotate(ghost_model_matrix, glm::radians(angle), glm::vec<3, double>(0.0, 0.0, 1.0));
        sponza_model_matrix = glm::rotate(sponza_model_matrix, glm::radians(angle * 10.0), glm::vec<3, double>(0.0, 1.0, 0.0));

        // myRenderer.updateModel(ghostModelIndex, ghost_model_matrix);
        
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