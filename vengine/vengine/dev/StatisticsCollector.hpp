#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

class StatisticsCollector
{
private:
	IDXGIFactory* dxgiFactory;
	IDXGIAdapter* dxgiAdapter;
	IDXGIAdapter4* dxgiAdapter4;

	void ramUsage();
	void vramUsage();

public:
	StatisticsCollector();
	~StatisticsCollector();

	void update();
};