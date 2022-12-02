#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif //  WIN32 && _DEBUG

#include <cstdlib>

#include "vengine.h"
#include "vengine/test/TestScene2.hpp"

#ifdef WIN32 
#include "src/LevelEditorTestScene.h"
#endif


#include "src/TestDemoScene.h"
#include "src/NewTestScene.h"
#include "src/network/LobbyScene.h"
#include "src/network/NetworkHandlerTest.h"

int main(int argc, char* argv[])
{
	//  Set flags for tracking CPU memory leaks
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif //  _WIN32 && _DEBUG
    srand((unsigned int)time(0));

    {
        Engine engine;
        //engine.setCustomNetworkHandler(new NetworkHandlerTest());
        // engine.run("Demo Application", "assets/scripts/scene.lua", new TestScene2());
        //engine.run("Demo Application", "", new LevelEditorTestScene());
       engine.run("Demo Application", "assets/scripts/scene.lua", new TestDemoScene());
    }

	return EXIT_SUCCESS;
}
