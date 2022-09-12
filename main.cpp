#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) && defined(_DEBUG)
#endif // WIN32 && _DEBUG
#include <crtdbg.h>

#include <cstdlib>

#include "tracy/Tracy.hpp"
#include "src/Engine.hpp"
#include "src/TestScene.hpp"
#include <iostream>
int main(int argc, char* argv[])
{
    // Set flags for tracking CPU memory leaks
#if defined(WIN32) && defined(_DEBUG)
#endif // WIN32 && _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        struct s
        {
            int i;
            std::string str;
        };

        s* test = new s();
    }

    std::cout << "test" << std::endl;

    // _CrtDumpMemoryLeaks();
    return 0;
#if defined(WIN32) && defined(_DEBUG)
#endif // WIN32 && _DEBUG

    {
        Engine engine;
        engine.run(new TestScene()); 
    }

    return EXIT_SUCCESS;
}
