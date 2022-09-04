#include <stdexcept>
#include "Engine/Engine.h"
#include "Engine/DataStructures/BSP.h"
#include "Engine/Application/Time.h"
#include "Engine/Application/Input.h"
int main(int argn, char** argv)
{
	// Set flags for tracking §CPU memory leaks
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// Create engine within it's own scope
	{
		Engine engine;
		//engine.init();
        // Init
        engine.window.init(engine.renderer, 1280, 720);
        engine.renderer.init();
        Camera camera(engine.renderer);

        // Mesh data to render
        MeshData meshData;
        //meshData.loadOBJ("Resources/Models/dragon_vrip_res1.obj");
        //meshData.loadOBJ("Resources/Models/dragon_vrip_res4.obj");
        //meshData.loadOBJ("Resources/Models/dragon_vrip_res4_big.obj");
        meshData.loadOBJ("Resources/Models/suzanne.obj");
        //meshData.loadOBJ("Resources/Models/sphereTest.obj");
        //meshData.loadOBJ("Resources/Models/lowResSphere.obj");
        //meshData.loadOBJ("Resources/Models/lowResThreeSpheres.obj");
        //meshData.loadOBJ("Resources/Models/torus.obj");
        //meshData.create(quadsVertices, quadsIndices);

        // BSP to render mesh with
        BSP bsp;
        bsp.createFromMeshData(meshData);

        // Randomly create trees and choose the best one
        /*BSP* bsp = nullptr;
        BSP bsps[2]{};
        uint32_t currentIndex = 0;
        uint32_t lowestDepth = ~0u;
        for (uint32_t i = 0; i < 40; ++i)
        {
            MeshData tempMeshData; 
            tempMeshData.loadOBJ("Resources/Models/dragon_vrip_res4.obj");
            bsps[currentIndex].createFromMeshData(tempMeshData);

            uint32_t treeDepth = bsps[currentIndex].getTreeDepth();
            if (treeDepth < lowestDepth)
            {
                Log::write("Switched BSP");

                lowestDepth = treeDepth;
                bsp = &bsps[currentIndex];
                meshData = tempMeshData;
                currentIndex = (currentIndex + 1) % 2;
            }
        }
        Log::write("Chosen tree depth: " + std::to_string(lowestDepth));*/


        // Mesh to render
        Mesh mesh(engine.renderer);
        mesh.createMesh(meshData, true);

        bool wireframe = false;

        // Imgui
        /*IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void) io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window.getWindowHandle(), true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = renderer.getVkInstance();
        initInfo.PhysicalDevice = renderer.getVkPhysicalDevice();
        initInfo.Device = renderer.getVkDevice();
        initInfo.QueueFamily = renderer.getQueueFamilies().getIndices().graphicsFamily.value();
        initInfo.Queue = renderer.getQueueFamilies().getVkGraphicsQueue();
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = renderer.getImguiDescriptorPool().getVkDescriptorPool();
        initInfo.Allocator = nullptr;
        initInfo.MinImageCount = renderer.getSwapchain().getMinImageCount();
        initInfo.ImageCount = renderer.getSwapchain().getImageCount();
        initInfo.CheckVkResultFn = ;
        ImGui_ImplVulkan_Init(&initInfo, );*/

        // Main loop
        Time::init();
        while (engine.window.isRunning())
        {
            // Update before "game logic"
            engine.window.update();
            Time::updateDeltaTime();

            // "Game logic"
            camera.update();
            bsp.traverseTree(meshData, camera.getPosition());
            mesh.getIndexBuffer().updateIndexBuffer(
                meshData.getIndices(), 
                engine.renderer.getCurrentFrameIndex()
            );

            if (Input::isKeyPressed(Keys::R))
            {
                wireframe = !wireframe;
                engine.renderer.setToWireframe(wireframe);
            }

            

            // Render
            // TODO: change to scene submission rather
            // than mesh submission
            engine.renderer.drawFrame(camera, mesh);

            // Print fps
            if (Time::hasOneSecondPassed())
                Log::write("FPS: " + std::to_string(1.0f / Time::getDT()));
        }

        // Cleanup
        engine.renderer.startCleanup();

        mesh.cleanup();

        engine.renderer.cleanup();
	}

#ifdef _DEBUG
	getchar();
#endif

	return EXIT_SUCCESS;
}