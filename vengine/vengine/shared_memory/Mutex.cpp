#include "Mutex.h"

Mutex::Mutex(LPCWSTR mutexName)
{	
	this->mutexHandle = CreateMutex(nullptr, false, mutexName);
	if (mutexHandle == NULL)
	{
		std::cout << "Failed to create mutex object" << std::endl;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		std::cout << "Mutex already exists" << std::endl;
	}
}

Mutex::~Mutex()
{
	CloseHandle(mutexHandle);
}

void Mutex::Lock()
{
	WaitForSingleObject(this->mutexHandle, INFINITE);
}

void Mutex::Unlock()
{
	ReleaseMutex(this->mutexHandle);
}
