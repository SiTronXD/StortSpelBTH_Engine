#pragma once

#include "Shader.h"

class VertexShader : public Shader
{
private:
public:
	VertexShader(Renderer& renderer);
	VertexShader(Renderer& renderer, const std::string& filePath);
	~VertexShader();

	void createFromFile(const std::string& filePath);
};