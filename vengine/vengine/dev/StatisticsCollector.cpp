#include "pch.h"
#include "StatisticsCollector.hpp"
#include "Log.hpp"

#include <psapi.h>
#include <string>

void StatisticsCollector::ramUsage()
{
	//src: https://learn.microsoft.com/en-us/windows/win32/api/psapi/ns-psapi-process_memory_counters

	DWORD currentProcessID = GetCurrentProcessId();

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, currentProcessID);

	if (NULL == hProcess)
		return;

	// https://learn.microsoft.com/en-us/windows/win32/psapi/process-memory-usage-information

	PROCESS_MEMORY_COUNTERS pmc{};
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		// PagefileUsage is the:
		// The Commit Charge value in bytes for this process.
		// Commit Charge is the total amount of memory that the memory manager has committed for a running process.

		float memoryUsage = float(pmc.PagefileUsage / 1024.0 / 1024.0); //MiB

		Log::write("RAM: " + std::to_string(memoryUsage) + " MiB");
	}

	CloseHandle(hProcess);
}

void StatisticsCollector::vramUsage()
{
	DXGI_QUERY_VIDEO_MEMORY_INFO info{};
	if (SUCCEEDED(dxgiAdapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info)))
	{
		float memoryUsage = float(info.CurrentUsage / 1024.0 / 1024.0); //MiB

		Log::write("VRAM: " + std::to_string(memoryUsage) + " MiB");
	}
}

StatisticsCollector::StatisticsCollector()
	: dxgiFactory(nullptr),
	dxgiAdapter(nullptr),
	dxgiAdapter4(nullptr)
{
	// Create factory
	HRESULT ret_code = ::CreateDXGIFactory(
		__uuidof(IDXGIFactory),
		reinterpret_cast<void**>(&this->dxgiFactory));
	if (SUCCEEDED(ret_code))
	{
		// Enumerate adapters
		if (SUCCEEDED(this->dxgiFactory->EnumAdapters(0, &this->dxgiAdapter)))
		{
			// Query interface
			if (!SUCCEEDED(this->dxgiAdapter->QueryInterface(__uuidof(IDXGIAdapter4), (void**) &this->dxgiAdapter4)))
			{
				Log::error("StatisticsCollector: Could not query interface from IDXGIAdapter4.");
			}
		}
	}
}

StatisticsCollector::~StatisticsCollector()
{
	this->dxgiAdapter4->Release();
	this->dxgiAdapter->Release();
	this->dxgiFactory->Release();

	this->dxgiAdapter4 = nullptr;
	this->dxgiAdapter = nullptr;
	this->dxgiFactory = nullptr;
}

void StatisticsCollector::update()
{
	this->ramUsage();
	this->vramUsage();
}