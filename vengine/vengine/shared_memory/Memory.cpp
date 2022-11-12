#include "pch.h"
#include "Memory.h"

Memory::Memory(LPCWSTR bufferName, size_t bufferSize)
{
	this->bufferSize = bufferSize;

	this->controlbufferSize = sizeof(ControlHeader);
	this->controlbufferName = L"CtrlMap";

	InitializeFilemap(bufferName);
	InitializeFileview();
}

Memory::~Memory()
{
	UnmapViewOfFile(memoryData);
	CloseHandle(memoryFilemap);

	UnmapViewOfFile(controlData);
	CloseHandle(controlFilemap);
}

void Memory::InitializeFilemap(LPCWSTR buffername)
{
	//Try to create file mapping object for memorybuffer
	memoryFilemap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
		0, bufferSize, buffername);

	std::cout << "Creating file map for memory buffer with size " << bufferSize << std::endl;

	if (memoryFilemap == NULL)
		std::cout << "Failed to create file mapping object for memorybuffer" << std::endl;

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		std::cout << "File mapping object already exists" << std::endl;

	//Try to create file map for controlbuffer
	controlFilemap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, controlbufferSize, controlbufferName);
	if (controlFilemap == NULL)
	{
		std::cout << "Failed to create file mapping object for controlbuffer" << std::endl;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		std::cout << "File mapping object already exists" << std::endl;
	}
}

void Memory::InitializeFileview()
{
	memoryData = (char*)MapViewOfFile(memoryFilemap, FILE_MAP_ALL_ACCESS, 0, 0, bufferSize);
	if (memoryData == NULL)
		std::cout << "View of file mapping object for memorydata failed" << std::endl;

	controlData = (size_t*)MapViewOfFile(controlFilemap, FILE_MAP_ALL_ACCESS, 0, 0, controlbufferSize);
	if (controlData == NULL)
		std::cout << "View of file mapping object for controldata failed" << std::endl;
}