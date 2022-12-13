#include "pch.h"
#if defined(_WIN32)
#include "StatisticsCollector.hpp"
#include "../dev/Log.hpp"

#include <psapi.h>
#include <string>

StatisticsCollector::StatisticsCollector()
	: dxgiFactory(nullptr),
	dxgiAdapter(nullptr),
	dxgiAdapter4(nullptr)
{
	// ---------- Objects for VRAM monitoring ----------

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
			if (!SUCCEEDED(this->dxgiAdapter->QueryInterface(__uuidof(IDXGIAdapter4), (void**)&this->dxgiAdapter4)))
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

float StatisticsCollector::getRamUsage()
{
	float memoryUsage = -1.0f;

	//src: https://learn.microsoft.com/en-us/windows/win32/api/psapi/ns-psapi-process_memory_counters

	DWORD currentProcessID = GetCurrentProcessId();

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, currentProcessID);

	if (NULL == hProcess)
		return memoryUsage;

	// https://learn.microsoft.com/en-us/windows/win32/psapi/process-memory-usage-information

	PROCESS_MEMORY_COUNTERS pmc{};
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		// PagefileUsage is the:
		// The Commit Charge value in bytes for this process.
		// Commit Charge is the total amount of memory that the memory manager has committed for a running process.

		memoryUsage = float(pmc.PagefileUsage / 1000.0 / 1000.0); // MB
	}

	CloseHandle(hProcess);

	return memoryUsage;
}

float StatisticsCollector::getVramUsage()
{
	float memoryUsage = -1.0f;

	DXGI_QUERY_VIDEO_MEMORY_INFO info{};
	if (SUCCEEDED(dxgiAdapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info)))
	{
		memoryUsage = float(info.CurrentUsage / 1000.0 / 1000.0); // MB
	}

	return memoryUsage;
}

#endif