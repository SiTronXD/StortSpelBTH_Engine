#include <iostream>
#include <comdef.h>
#include "Log.h"

void Log::write(const std::string& message)
{
	std::cout << "[Log]: " << message << std::endl;
}

void Log::warning(const std::string& message)
{
	std::cout << "[Log Warning]: " << message << std::endl;
}

void Log::error(const std::string& errorMessage)
{
	// Convert const char* to LPCWSTR
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, errorMessage.c_str(), -1, wString, 4096);

	// Simple message box
	MessageBox(
		NULL, wString, L"ERROR", MB_OK
	);

	delete[] wString;
}