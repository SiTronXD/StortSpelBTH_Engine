#pragma once 
 #include "op_overload.hpp"

#include <vector>
#include <string>
#include "../../application/SceneHandler.hpp"
#include "../../components/Script.hpp"
#include "../../components/MeshComponent.hpp"
#include "../LuaPushes.hpp"

class SceneLua
{
private:
	// COUNT: Getting the number of Components
	enum class CompType { TRANSFORM, MESH, SCRIPT, CAMERA, COLLIDER, RIGIDBODY, ANIMATION, UIAREA, AUDIOSOURCE, COUNT };
	inline static const std::vector<std::string> compTypes {
		"Transform",
		"Mesh",
		"Script",
		"Camera",
		"Collider",
		"Rigidbody",
		"Animation",
		"UIArea",
		"AudioSource"
	};

	// COUNT: Getting the number of Systems
	enum class SystemType { COUNT };
	inline static const std::vector<std::string> systemTypes {
	};

	static int lua_createSystem(lua_State* L);
	static int lua_setScene(lua_State* L);
	static int lua_iterateView(lua_State* L);
	static int lua_createPrefab(lua_State* L);

	static int lua_getMainCamera(lua_State* L);
	static int lua_setMainCamera(lua_State* L);

	static int lua_getEntityCount(lua_State* L);
	static int lua_entityValid(lua_State* L);
	static int lua_createEntity(lua_State* L);
	static int lua_removeEntity(lua_State* L);

	static int lua_hasComponent(lua_State* L);
	static int lua_getComponent(lua_State* L);
	static int lua_setComponent(lua_State* L);
	static int lua_removeComponent(lua_State* L);

	static int lua_setActive(lua_State* L);
	static int lua_setInactive(lua_State* L);
	static int lua_isActive(lua_State* L);
	static int lua_setAnimation(lua_State* L);
	static int lua_blendToAnimation(lua_State* L);
	static int lua_syncedBlendToAnimation(lua_State* L);
	static int lua_getAnimationSlot(lua_State* L);
	static int lua_setAnimationSlot(lua_State* L);
	static int lua_setAnimationTimeScale(lua_State* L);
	static int lua_getAnimationStatus(lua_State* L);

   public:
	static void lua_openscene(lua_State* L, SceneHandler* sceneHandler);
};