#pragma once

#include <entt.hpp>

class System
{
private:
public:
	// true: destroy system, false: continue
	virtual bool update(entt::registry& reg, float deltaTime) = 0;
};