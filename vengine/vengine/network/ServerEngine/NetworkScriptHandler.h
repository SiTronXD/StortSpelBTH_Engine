#pragma once

#include <string>
#include <vector>
#include "../../dev/Fail.h"
#include "../../components/Script.hpp"
#include "../../lua/ScriptHandler.h"
#include <ios>

class NetworkScriptHandler : public ScriptHandler
{
   private:
	//deleted
	template <typename T = bool>
	void updateScripts()
	{
		static_assert(fail<T>::value, "Do not use this function! Use updateScripts(float dt)");
	}
	void updateScripts(float dt);
	
   public:
	NetworkScriptHandler();
	virtual ~NetworkScriptHandler();
	void updateSystems(std::vector<LuaSystem>& vec, float dt);
	void update(float dt);


	//deleted functions
	//normal update doesn't work in network
	//deleted
	template <typename T = bool>
	void update()
	{
		static_assert(fail<T>::value, "Do not use this function! Use update(float dt)");
	}
	//deleted
	template <typename T = bool>
	void updateSystems(std::vector<LuaSystem>& vec)
	{
		static_assert(fail<T>::value, "Do not use this function! Use updateSystems(std::vector<LuaSystem>& vec, float dt)");
	}

};