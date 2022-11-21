#pragma once

#include <entt.hpp>

class System
{
private:
public:
	virtual ~System(){}

	// true: destroy system, false: continue
	virtual bool update(entt::registry& reg, float deltaTime) = 0;
};