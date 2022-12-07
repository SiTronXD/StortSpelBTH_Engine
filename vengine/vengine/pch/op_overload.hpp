#pragma once 


#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <new>
#include <string>
#include <map>      // Note: unordered_map did not work, caused errors randomly (maybe caused by tracer running in seperate thread)
#include <sstream> 
//#include <array>
#include <vector>

struct allocInfo
{
    allocInfo(const std::string& str, void* alloc):identification(str),alloc(alloc){}
    std::string identification; 
    void* alloc = nullptr;
    ~allocInfo(){};
}; 


void* registerAlloc(std::size_t size, char const* sourceFile, uint32_t lineNr);

void* registerAlloc(std::size_t size);

// void* operator new(__FILE__, __LINE__)(std::size_t size)
// {
//     return registerAlloc(size);    
// }

// Overload new(__FILE__, __LINE__) operators
void* operator new(std::size_t size, char const* sourceFile, uint32_t lineNr);

void* operator new[](std::size_t size, char const* sourceFile, uint32_t lineNr);

// Overload delete operator
void operator delete(void* ptr) noexcept

    ;

// #define new new(__FILE__, __LINE__)

extern std::vector<void* >* allShit;

void reportMemoryLeaks();

void deleteMemoryLeakMap();