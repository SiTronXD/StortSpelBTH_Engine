#pragma once 
 #include "op_overload.hpp"

#if defined(_WIN32)
#include <d3d11_4.h>
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

class StatisticsCollector
{
private:
	IDXGIFactory* dxgiFactory;
	IDXGIAdapter* dxgiAdapter;
	IDXGIAdapter4* dxgiAdapter4;

public:
	StatisticsCollector();
	~StatisticsCollector();

	float getRamUsage();
	float getVramUsage();
};


#endif