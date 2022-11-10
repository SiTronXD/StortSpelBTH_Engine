#include "pch.h"
#include "StringHelper.hpp"

void StringHelper::splitString(
	std::string& str,
	const char& delim,
	std::vector<std::string>& output)
{
	size_t pos = 0;
	while ((pos = str.find(delim)) != std::string::npos)
	{
		// Extract and add token
		output.emplace_back(str.substr(0, pos));

		// Remove front
		str.erase(0, pos + 1);
	}

	// Add remaining string
	output.emplace_back(str);
}

void StringHelper::splitString(
	const std::string& str, 
	const char& delim, 
	std::vector<std::string>& output)
{
	size_t pos = 0;
	std::string strCopy = str;
	while ((pos = strCopy.find(delim)) != std::string::npos)
	{
		// Extract and add token
		output.emplace_back(strCopy.substr(0, pos));

		// Remove front
		strCopy.erase(0, pos + 1);
	}

	// Add remaining string
	output.emplace_back(strCopy);
}