#pragma once

#include <string>
#include <map>
#ifdef WIN32
#include <Windows.h>
#endif

#include "glm/glm.hpp"

struct Filter 
{
    bool value = true;
};

class Log
{
private:
    static std::map<std::string, Filter> filters;
public:
    static void addFilter(const std::string& filterName);

	static void write(const std::string& message, const std::string& filter = "");
	static void warning(const std::string& message, const std::string& filter = "");
	static void error(const std::string& errorMessage, const std::string& filter = "");
	static std::string vecToStr(const glm::vec3& vec);
};
