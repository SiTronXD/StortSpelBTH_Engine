#pragma once

#include <string>
#ifdef WIN32
#include <Windows.h>
#endif

class Log
{
public:
	static void write(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& errorMessage);
};
