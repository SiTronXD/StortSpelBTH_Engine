#pragma once
#include <memory>
/// This will later be replaced by a class that parses a .cfg/.ini-file with these values
/// in order to easily change constants



// const std::string PATH_ASSETS    =   "assets/";
// const std::string PATH_MODELS    =   PATH_ASSETS+"models/";
// const std::string PATH_TEXTURES  =   PATH_ASSETS+"textures/";
// const std::string PATH_SHADERS   =   PATH_ASSETS+"shaders/";


/// Custom Allocator Functions, used to enable Tracy Memory Profiling...

void* CustomAlloc( size_t count );
void CustomFree( void* ptr );