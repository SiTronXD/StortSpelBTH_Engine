#include "op_overload.hpp" 

static std::map<intptr_t,allocInfo>* allocIdentifications= new std::map<intptr_t,allocInfo>();

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
    std::string fileName;
    //std::stringstream fileName;

    std::cout << "sourceFile"
              << ":"
              << "lineNr"
              << " " << alloc << std::endl;
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
void operator delete(void* ptr) noexcept

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
    std::free(ptr);
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
    delete allocIdentifications;
    allocIdentifications = nullptr;
}