#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

#include <cstdlib>
#include "assimp/aabb.h"
#include <assimp/aabb.h>
#include "tracy/Tracy.hpp"
#include "src/Engine.h"
#include "src/TestScene.h"

int main(int argc, char* argv[])
{
    // Set flags for tracking CPU memory leaks
#if defined(WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // WIN32 && _DEBUG

    {
        Engine engine;
        engine.run(new TestScene()); 
    }

    _CrtDumpMemoryLeaks();

    return EXIT_SUCCESS;
}
