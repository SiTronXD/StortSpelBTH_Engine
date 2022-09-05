#include "VulkanRenderer.h" /// Use the 3D_VEngine renderer
#include "imgui.h"          /// If Dear ImGui is used
#include <chrono>
#include <iostream> 

int main() {

    /// Creating Vulkan Renderer Instance
    auto myRenderer = VulkanRenderer();
    if (myRenderer.init("Some Program") == EXIT_FAILURE ) {
        return EXIT_FAILURE;
    }

    /// Creating a function (in this case, a lambda) which will be our gameloop
    auto lambda_game_loop = [&](SDL_Events& events)
    {    
        /// Use IMGUI anywhere this function/lambda is a parent too...
        ImGui::Begin("welcome");        
        ImGui::Text("Hello World!");
        ImGui::End();      
        
    };
    /// Register gameloop function, renderer will start immediately once this is called
    myRenderer.registerGameLoop(lambda_game_loop);

    /// To avoid memory leak. run the cleanup method...
    myRenderer.cleanup();
 
    return EXIT_SUCCESS;
}