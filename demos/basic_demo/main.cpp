/*#include "VulkanRenderer.h" /// Use the 3D_VEngine renderer
#include "imgui.h"          /// If Dear ImGui is used
#include "glm/common.hpp"   /// If glm math is used 
#include "SDL2/SDL.h"       /// If SDL events are used
#include <chrono>
#include <iostream> 

int main() {

    /// Creating Vulkan Renderer Instance
    auto myRenderer = VulkanRenderer();
    if (myRenderer.init("Some Program") == EXIT_FAILURE ) {
        return EXIT_FAILURE;
    }

    /// Load a Model
    int model_id = myRenderer.createModel("ghost.obj");

    // Prepare a matrix, used when updating the model
    glm::mat4 ghost_mat(1.f); // Basic Identity Matrix

    /// Prepare deltatime variable
    auto lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> deltaTime{};

    /// Creating a function (in this case, a lambda) which will be our gameloop
    auto lambda_game_loop = [&](SDL_Events& events)
    {
        /// Handle any events exposed by SDL
        for(auto&& event : events){
            if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                    case SDLK_f:
                        std::cout << "Hello" << "\n"; // Press F to say Hello
                    default: ;
                }
            }
        }

        /// Update Deltatime
        auto time_now = std::chrono::high_resolution_clock::now();
        deltaTime = time_now - lastTime;
        lastTime = time_now; 

        /// Use IMGUI anywhere this function/lambda is a parent too...
        ImGui::Begin("welcome");        
        ImGui::Text("Hello World!");
        ImGui::End();      

        /// Manipulate Ghost Matrix 
        ghost_mat = glm::rotate(ghost_mat, (float)glm::radians(5.f * deltaTime.count()), glm::vec3(0.f,1.f,0.f));

        /// Update Ghost 
        myRenderer.updateModel(model_id, ghost_mat); 
    };
    /// Register gameloop function, renderer will start immediately once this is called
    myRenderer.registerGameLoop(lambda_game_loop);

    /// To avoid memory leak. run the cleanup method...
    myRenderer.cleanup();
 
    return EXIT_SUCCESS;
}*/