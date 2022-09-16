#pragma once

#include <chrono>

//same thing as Time but can make multiple instances
class Timer
{
private:

	float deltaTime;

	float multiplier;

	std::chrono::system_clock::time_point lastTime;
	std::chrono::system_clock::time_point currentTime;
	std::chrono::duration<float> elapsedSeconds;

public:
	Timer();
	void updateDeltaTime();
	void ChangeTimeFlow(float multiplier);

	//return deltaTime with a multiplier
	const inline float& getDT() { return deltaTime * multiplier; };
	//return the real deltaTime
	const inline float& getRealDT() { return deltaTime; };
};