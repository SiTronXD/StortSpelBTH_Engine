#pragma once
#include <string>

class Material
{
private:
	std::string name;
	float ambient[3];
	int diffuseTextureID;
	int normalTextureID;

public:
	Material();

};