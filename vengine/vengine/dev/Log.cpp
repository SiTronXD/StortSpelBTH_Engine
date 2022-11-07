#include "pch.h"
#ifdef WIN32
#include <comdef.h>
#endif 

#include <iostream>
#include "Log.hpp"
#include <cassert>


void Log::write(const std::string& message)
{
	std::cout << "[Log]: " << message << std::endl;
}

void Log::warning(const std::string& message)
{
	std::cout << "[Log Warning]: " << message << std::endl;
}

#ifdef _WIN32
#include <Windows.h>
void Log::error(const std::string& errorMessage)
{
	std::cout << "[Log Error]: " << errorMessage << std::endl;

	// Convert const char* to LPCWSTR
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, errorMessage.c_str(), -1, wString, 4096);

	// Simple message box
	MessageBox(
		NULL, wString, L"ERROR", MB_OK
	);

	delete[] wString;
}
#else
void Log::error(const std::string& message)
{
	std::cout << "[Log Error]: " << message << std::endl;
    assert(false);
}
#endif
