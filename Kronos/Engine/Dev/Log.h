#pragma once

#include <string>
#include <Windows.h>

class Log
{
public:
	static void write(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& errorMessage);
};
