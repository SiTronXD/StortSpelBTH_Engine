#include <cstdlib>

#include "src/Engine.h"
#include "src/TestScene.h"

int main(int argc, char* argv[])
{
    {
        Engine engine;
        engine.run(new TestScene(engine.sceneHandler)); 
    }

    return EXIT_SUCCESS;
}
