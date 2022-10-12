#include "Timer.h"

Timer::Timer()
{
    multiplier = 1;
    lastTime   = std::chrono::system_clock::now();

    updateDeltaTime();
}

void Timer::updateDeltaTime()
{
    // Update elapsed time
    currentTime    = std::chrono::system_clock::now();
    elapsedSeconds = currentTime - lastTime;
    lastTime       = currentTime;

    // Update delta time
    deltaTime = elapsedSeconds.count();
}

void Timer::ChangeTimeFlow(const float& multiplier)
{
    this->multiplier = multiplier;
}
