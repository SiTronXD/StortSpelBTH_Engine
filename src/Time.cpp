#include "Time.h"

float Time::deltaTime = 0.0f;
float Time::timeSinceStart = 0.0f;
bool Time::oneSecondPassed = false;

std::chrono::system_clock::time_point Time::lastTime = std::chrono::system_clock::time_point();
std::chrono::system_clock::time_point Time::currentTime = std::chrono::system_clock::time_point();
std::chrono::duration<float> Time::elapsedSeconds = std::chrono::duration<float>();

void Time::init()
{
	lastTime = std::chrono::system_clock::now();
	updateDeltaTime();
}

void Time::updateDeltaTime()
{
	// Update elapsed time
	currentTime = std::chrono::system_clock::now();
	elapsedSeconds = currentTime - lastTime;
	lastTime = currentTime;

	// Update delta time
	deltaTime = elapsedSeconds.count();

	// Update time since start
	unsigned int beforeTime = (unsigned int) timeSinceStart;
	timeSinceStart += deltaTime;

	// Update if a second has passed
	oneSecondPassed = beforeTime != (unsigned int) timeSinceStart;
}
