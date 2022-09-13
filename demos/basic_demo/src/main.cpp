#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif // WIN32 && _DEBUG

#include <cstdlib>

#include "vengine.h"
#include "tracy/Tracy.hpp"


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
#if defined(WIN32) && defined(_DEBUG)
    _CrtDumpMemoryLeaks();
#endif // WIN32 && _DEBUG

    return EXIT_SUCCESS;
}
