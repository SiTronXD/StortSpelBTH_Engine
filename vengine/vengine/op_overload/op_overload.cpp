#include "pch.h"
#include "op_overload.hpp" 

static std::map<intptr_t,allocInfo>* allocIdentifications= new std::map<intptr_t,allocInfo>();

std::vector<void* >* allShit = nullptr;



std::array<MyData ,1000000> shits;
int nrShits = 0;
bool was_last_ = false;
bool initialized_ = false;

void* registerAlloc(std::size_t size, char const* sourceFile, uint32_t lineNr)
{
    
    void* alloc = std::malloc(size);
    std::stringstream fileName;

    // Prepare output for Memory leaks, content to write if leak on alloc happens
    fileName << sourceFile << ":" << lineNr << " " << alloc << std::endl;

    // Store Allocation information, for a specific allocation (use int representation of addr for void*)
    allocIdentifications->insert(
        {reinterpret_cast<intptr_t>(alloc), allocInfo(fileName.str(), alloc)}
    );

    return alloc;
}
void* registerAlloc(std::size_t size)
{

    void* alloc = std::malloc(size);
    //std::string fileName;
    ////std::stringstream fileName;

    //std::cout << "sourceFile"
    //          << ":"
    //          << "lineNr"
    //          << " " << alloc << std::endl;
    // Prepare output for Memory leaks, content to write if leak on alloc happens
    //fileName << "sourceFile" << ":" << "lineNr" << " " <<  alloc << std::endl;

    // Store Allocation information, for a specific allocation (use int representation of addr for void*)

    return alloc;
}
void* operator new(std::size_t size, char const* sourceFile, uint32_t lineNr)
{
    return registerAlloc(size, sourceFile, lineNr);
}
void* operator new[](std::size_t size, char const* sourceFile, uint32_t lineNr)
{
    return registerAlloc(size, sourceFile, lineNr);
}

#include <Windows.h>
#include <dbghelp.h>

struct StackTrace {
    private:
        enum { NFrames = 20};
        int m_frameCount;
        void* m_frames[NFrames];
    public:
        StackTrace() {
            m_frameCount = CaptureStackBackTrace(1, NFrames, m_frames, NULL);
        }
};



void printStacktrace(int allocID ) {
    // http://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb204633(v=vs.85).aspx

    //unsigned int   i;
    //void         * stack[ 10 ];
    //unsigned short frames;
    //SYMBOL_INFO  * symbol;
    //HANDLE         process;

    //process = GetCurrentProcess();

    //SymInitialize( process, NULL, TRUE );

    //frames               = CaptureStackBackTrace( 0, 10 , stack, NULL );
    ////symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 1256 * sizeof( char ), 1 );
    ////symbol->MaxNameLen   = 255;
    ////symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    ////printf("\nStacktrace\n");

    //for( i = 0; i < frames; i++ ) {
    //    //SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

    //    //printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
    //    /*char buf[10000];
    //    sprintf(buf, "%i: %s - 0x%0X\n\0", frames - i - 1, symbol->Name, symbol->Address );

    //    strncpy(shits[nrShits].stacktace, buf,  1000);*/

    //}

    //free( symbol );
}

void* operator new(std::size_t size)
{
    
    shits[nrShits].address = std::malloc(size);
    //auto a = StackTrace();
    //print_symbol_info(shits[nrShits]);
    if(initialized_)
    {
        printStacktrace(nrShits);
    }
    
    return shits[nrShits++].address;
        
}

void operator delete(void* ptr) noexcept

{

    if(!was_last_)
    {
        // Avoid constexpr allocations
        if (allocIdentifications != nullptr && allocIdentifications->size() != 0 &&
            allocIdentifications != ptr)
            {
                auto key = reinterpret_cast<intptr_t>(ptr);
                if (allocIdentifications->find(key) != allocIdentifications->end())
                    {
                        // Set maps ptr for key to nullptr, indicate it is freed!
                        allocIdentifications->at(reinterpret_cast<intptr_t>(ptr))
                            .alloc = nullptr;
                    }
            }
    }
    
    std::free(ptr);
}
void operator delete[](void* ptr) noexcept

{

    if(!was_last_)
    {
        // Avoid constexpr allocations
        if (allocIdentifications != nullptr && allocIdentifications->size() != 0 &&
            allocIdentifications != ptr)
            {
                auto key = reinterpret_cast<intptr_t>(ptr);
                if (allocIdentifications->find(key) != allocIdentifications->end())
                    {
                        // Set maps ptr for key to nullptr, indicate it is freed!
                        allocIdentifications->at(reinterpret_cast<intptr_t>(ptr))
                            .alloc = nullptr;
                    }
            }
    }
    
    std::free(ptr);
}

void operator delete(void* ptr, char const* sourceFile, uint32_t lineNr) noexcept

{

    if(!was_last_)
    {
        // Avoid constexpr allocations
        if (allocIdentifications != nullptr && allocIdentifications->size() != 0 &&
            allocIdentifications != ptr)
            {
                auto key = reinterpret_cast<intptr_t>(ptr);
                if (allocIdentifications->find(key) != allocIdentifications->end())
                    {
                        // Set maps ptr for key to nullptr, indicate it is freed!
                        allocIdentifications->at(reinterpret_cast<intptr_t>(ptr))
                            .alloc = nullptr;
                    }
            }
    }
    
    std::free(ptr);
}

void initMe()
{
    allShit = new std::vector<void*>();
    initialized_ = true;
}

void reportMemoryLeaks()
{

    std::cout << "\nMemory Leaks: \n";
    for (auto usedAddress : *allocIdentifications)
        {
            allocInfo& info = usedAddress.second;
            if (info.alloc != nullptr)
                {
                    std::cout << info.identification;
                }
        }
    std::cout << "\n";
    deleteMemoryLeakMap();
}
void deleteMemoryLeakMap()
{
    was_last_ = true;
    delete allocIdentifications;
    allocIdentifications = nullptr;
}