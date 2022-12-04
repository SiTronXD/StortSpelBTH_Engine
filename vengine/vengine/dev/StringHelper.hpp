#pragma once 
 #include "op_overload.hpp"

#include <string>
#include <vector>

class StringHelper
{
public:
	static void splitString(std::string& str, const char& delim,
		std::vector<std::string>& output);
};