#pragma once

#include <string>
#ifdef WIN32
#include <Windows.h>
#endif

#include "glm/glm.hpp"

class Log
{
public:
	static void write(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& errorMessage);
	static std::string vecToStr(const glm::vec3& vec);
};
