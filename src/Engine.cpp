#include "Engine.h"

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

///We need to take care of ALL error messages... vulkan does not report by default...
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <vector>

#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "Configurator.h"

#include <chrono>
#include <functional>

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

    using namespace vengine_helper::config;
    loadConfIntoMemory(); /// load config data into memory

    /// Creating Vulkan Renderer Instance
    auto myRenderer = VulkanRenderer();
    if (myRenderer.init("Some Program") == 1) {
        std::cout << "EXIT_FAILURE" << std::endl;
        //return EXIT_FAILURE;
    }


    double angle = 0.F;

    int sponzaIndex = myRenderer.createModel("sponza.obj");
    int ghostModelIndex = myRenderer.createModel("ghost.obj");

    glm::mat4 correctSize = glm::mat4(1.F);
    float scale = 0.2F;
    correctSize = glm::scale(correctSize, glm::vec3(scale, scale, scale));

    glm::mat4 correctRot = glm::mat4(1.F);

    scale = 5.F;

    myRenderer.updateModel(ghostModelIndex, correctRot);
    correctRot = glm::translate(glm::mat4(1.F), glm::vec3(0.F, 10.F, -100.F));
    correctRot = glm::rotate(correctRot, glm::radians(-90.F), glm::vec3(1.F, 0.F, 0.F));
    correctRot = glm::rotate(correctRot, glm::radians(-90.F), glm::vec3(0.F, 0.F, 1.F));
    correctRot = glm::scale(correctRot, glm::vec3(scale, scale, scale));
    myRenderer.updateModel(ghostModelIndex, correctRot);

    auto lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> deltaTime{};


    auto lambda_game_loop = [&](SDL_Events& events)
    {
        this->sceneHandler.update();

        for (auto&& event : events) {
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_b:
                    std::cout << "Testing thing in loop" << "\n";
                default:;
                }
            }
        }

        auto time_now = std::chrono::high_resolution_clock::now();
        deltaTime = time_now - lastTime;
        lastTime = time_now;

        static bool open = true;
        ImGui::ShowDemoWindow(&open);

        ImGui::Begin("Another Window", &open);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            open = false;
        ImGui::End();

        angle += 10.0 * deltaTime.count();
        angle = angle > 360.0 ? 0.0 : angle;

        glm::mat<4, 4, double> ghost_model_matrix = correctRot;
        glm::mat<4, 4, double> sponza_model_matrix = correctSize;

        ghost_model_matrix = glm::rotate(ghost_model_matrix, glm::radians(angle), glm::vec<3, double>(0.0, 0.0, 1.0));
        sponza_model_matrix = glm::rotate(sponza_model_matrix, glm::radians(angle * 10.0), glm::vec<3, double>(0.0, 1.0, 0.0));


        myRenderer.updateModel(ghostModelIndex, ghost_model_matrix);
        myRenderer.updateModel(sponzaIndex, sponza_model_matrix);

        this->sceneHandler.updateToNextScene();
    };

    myRenderer.registerGameLoop(lambda_game_loop);

    myRenderer.cleanup();
}