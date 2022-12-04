#pragma once 
 #include "op_overload.hpp"
#include <Windows.h>
#include <iostream>

class Mutex
{
private:
	HANDLE mutexHandle;

public:
	Mutex(LPCWSTR mutexName);
	~Mutex();

	void Lock();
	void Unlock();
};
