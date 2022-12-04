#include "pch.h"
#ifdef WIN32
#include <comdef.h>
#endif 

#include <iostream>
#include "Log.hpp"
#include <cassert>

std::map<std::string, Filter> Log::filters;

void Log::write(const std::string& message, const std::string& filter)
{
	if (Log::filters[filter].value)
	{
		std::cout << "[Log]: " << message << std::endl;
	}
}

void Log::warning(const std::string& message, const std::string& filter)
{
	if (Log::filters[filter].value)
	{
		std::cout << "[Log Warning]: " << message << std::endl;
	}
}

#ifdef _WIN32
#include <Windows.h>
void Log::error(const std::string& errorMessage, const std::string& filter)
{
	if (Log::filters[filter].value)
	{
		std::cout << "[Log Error]: " << errorMessage << std::endl;
	
		// Convert const char* to LPCWSTR
		wchar_t* wString = new(__FILE__, __LINE__) wchar_t[4096];
		MultiByteToWideChar(CP_ACP, 0, errorMessage.c_str(), -1, wString, 4096);

		// Simple message box
		MessageBox(
			NULL, wString, L"ERROR", MB_OK
		);

		delete[] wString;
	}
}
#else
void Log::error(const std::string& message, const std::string& filter)
{
    if (Log::filters[filter].value)
	{
        std::cout << "[Log Error]: " << message << std::endl;
        assert(false);
    }
}
#endif

std::string Log::vecToStr(const glm::vec3& vec)
{
	return "(" + std::to_string(vec.x) + ", " + 
		std::to_string(vec.y) + ", " + 
		std::to_string(vec.z) + ")";
}

std::string Log::vecToStr(const glm::vec2& vec)
{
	return "(" + std::to_string(vec.x) + ", " +
		std::to_string(vec.y) + ")";
}

void Log::addFilter(const std::string& filterName)
{
    Log::filters.insert({filterName, {false}});
}
