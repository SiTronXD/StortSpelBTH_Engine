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

 void* operator new(std::size_t size);

// Overload new(__FILE__, __LINE__) operators
void* operator new(std::size_t size, char const* sourceFile, uint32_t lineNr);

void* operator new[](std::size_t size, char const* sourceFile, uint32_t lineNr);

// Overload delete operator
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, char const* sourceFile, uint32_t lineNr) noexcept;

// #define new new(__FILE__, __LINE__)

struct MyData
{
    void* address;
    char stacktace[1000];
};

extern std::vector<void* >* allShit;
extern std::array<MyData ,1000000> shits;
extern int nrShits;

extern bool was_last_;

void printStacktrace(int allocID );
void reportMemoryLeaks();
//void print_symbol_info(void* address);
//void load_symbol_info();

void deleteMemoryLeakMap();
void initMe();