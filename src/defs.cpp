#include "defs.h"
#include <string>
#include "tracy/Tracy.hpp"

/// Operator overloading of new, used to enable tracy memory profiling!
void* operator new( std::size_t count )
{
    std::unique_ptr<void*> f;
    auto *ptr = malloc( count );    //:NOLINT: overidden new operator, must use malloc
    TracySecureAlloc( ptr, count );
    return ptr;
}

void operator delete( void* ptr ) noexcept
{
    TracySecureFree( ptr );
    free( ptr );                    //:NOLINT: overidden delete operator, must use free
}

void* CustomAlloc( size_t count )
{
    auto *ptr = malloc( count );    //:NOLINT: replacement wrapper for malloc, 
    TracySecureAlloc( ptr, count);
    return ptr;
}

void CustomFree( void* ptr )
{
    TracySecureFree( ptr );
    free( ptr );                    //:NOLINT: replacement wrapper for malloc, should free
}
