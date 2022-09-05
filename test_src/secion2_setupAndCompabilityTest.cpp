#include "secion2_setupAndCompabilityTest.h"

void section2_setupAndCompatibilityTest() {
    std::cout << "Hello, World!!!" << std::endl;

    /// Initiates GLFW and vulkan
    glfwInit();

    /// Setting up details for our Window
    glfwWindowHint(GLFW_CLIENT_API,     /// By default this is set to OpenGL
                   GLFW_NO_API          /// We will use Vulkan so we set it to GLFW_NO_API
    );

    GLFWwindow *window = glfwCreateWindow(800,600, "My Test Window", nullptr, nullptr);

    /// Figure out how many Vulkan Extensions we can support.

    uint32_t extensionCount = 0;    /// uint32_t is actually a typedef for unsigned int...

    /// Get the Extension Count, store in extensionCount...
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    printf("Extension Count: %i\n",extensionCount);

    ///*
    /// Most Vulkan functions start with a lowercase 'vk'
    /// Most Vulkan types starts with a uppercase 'Vk'
    ///*//

    /// Testing if GLM is working...

    glm::mat4 testMatrix(1.0f);
    glm::vec4 testVector(1.0f);

    auto testResult = testMatrix * testVector;

    printf("GLM Test, Vector is: (%f,%f,%f)\n", testResult.x,testResult.y, testResult.z);


    while(!glfwWindowShouldClose(window))
    {

        glfwPollEvents();   /// Let's us checks if anything within the window has changed, such as if it was closed

    }

    glfwDestroyWindow(window);
    glfwTerminate();            ///Deactivate GLFW

    std::cout << "Bye, World!" << std::endl;
}
