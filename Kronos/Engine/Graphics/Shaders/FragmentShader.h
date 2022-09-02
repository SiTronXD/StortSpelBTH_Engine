#pragma once

#include "Shader.h"

class FragmentShader : public Shader
{
private:
public:
	FragmentShader(Renderer& renderer);
	FragmentShader(Renderer& renderer, const std::string& filePath);
	~FragmentShader();

	void createFromFile(const std::string& filePath);
};