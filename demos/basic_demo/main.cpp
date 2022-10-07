#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif // WIN32 && _DEBUG

#include <cstdlib>



#include "vengine.h"
#include "vengine/test/TestScene2.hpp"
#include "src/TestDemoScene.h"
#include "src/NetworkTestScene.h"

int main(int argc, char* argv[])
{
	// Set flags for tracking CPU memory leaks
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _WIN32 && _DEBUG

    {
        Engine engine;
        engine.run("Demo Application", "assets/scripts/scene.lua", new TestScene2());
    }

	return EXIT_SUCCESS;
}
