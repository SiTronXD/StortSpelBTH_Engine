#include <stdexcept>
#include "Engine/Engine.h"

int main()
{
	// Set flags for tracking CPU memory leaks
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// Create engine within it's own scope
	{
		Engine engine;
		engine.init();
	}

#ifdef _DEBUG
	getchar();
#endif

	return EXIT_SUCCESS;
}