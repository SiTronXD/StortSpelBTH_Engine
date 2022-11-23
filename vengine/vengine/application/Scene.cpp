#include "pch.h"
#include "Scene.hpp"
#include "SceneHandler.hpp"
#include "../network/NetworkHandler.h"
#include "../lua/ScriptHandler.h"
#include "../graphics/UIRenderer.hpp"
#include "../ai/AIHandler.hpp"
#include "Time.hpp"
#include "../components/AnimationComponent.hpp"

void Scene::switchScene(Scene* scene, std::string path)
{
	this->sceneHandler->setScene(scene, path);
}

NetworkHandler* Scene::getNetworkHandler()
{
	return this->sceneHandler->getNetworkHandler();
}

ScriptHandler* Scene::getScriptHandler()
{
	return this->sceneHandler->getScriptHandler();
}

ResourceManager* Scene::getResourceManager()
{
	return this->sceneHandler->getResourceManager();
}

PhysicsEngine* Scene::getPhysicsEngine()
{
	return this->sceneHandler->getPhysicsEngine();
}

UIRenderer* Scene::getUIRenderer()
{
	return this->sceneHandler->getUIRenderer();
}

DebugRenderer* Scene::getDebugRenderer()
{
	return this->sceneHandler->getDebugRenderer();
}

SceneHandler* Scene::getSceneHandler()
{
	return this->sceneHandler;
}

AIHandler* Scene::getAIHandler()
{
    return this->sceneHandler->getAIHandler();   
}

AudioHandler* Scene::getAudioHandler()
{
	return this->sceneHandler->getAudioHandler();
}


Scene::Scene()
	: sceneHandler(nullptr), mainCamera(-1)
{
	this->reg.clear();
}

Scene::~Scene()
{
	for (size_t i = 0; i < this->systems.size(); ++i)
	{
		delete this->systems[i];
	}
	this->reg.clear();
	this->systems.clear();
	this->luaSystems.clear();
}

Camera* Scene::getMainCamera()
{
	Camera* cam = nullptr;
	if (this->entityValid(this->mainCamera)) { cam = &this->getComponent<Camera>(this->mainCamera); }
	return cam;
}

Entity Scene::getMainCameraID()
{
	return this->mainCamera;
}

void Scene::setMainCamera(Entity entity)
{
	if (this->hasComponents<Camera>(entity)) { this->mainCamera = entity; }
}

void Scene::createSystem(std::string& path)
{
	this->luaSystems.push_back(LuaSystem { path, -1 });
}

void Scene::setScriptComponent(Entity entity, std::string path)
{
	this->getScriptHandler()->setScriptComponent(entity, path);
}

void Scene::updateSystems()
{
	for (auto it = this->systems.begin(); it != this->systems.end();)
	{
		if ((*it)->update(this->reg, Time::getDT()))
		{
			delete (*it);
			it = this->systems.erase(it);
		}
		else
		{
			it++;
		}
	}
}

int Scene::getEntityCount() const
{
	return (int)this->reg.alive();
}

bool Scene::entityValid(Entity entity) const
{
	return this->reg.valid((entt::entity)entity);
}

Entity Scene::createEntity()
{
	int entity = (int)this->reg.create();
	this->setComponent<Transform>(entity);
	return entity;
}

bool Scene::removeEntity(Entity entity)
{
	bool valid = this->entityValid(entity);
	if (valid) { this->reg.destroy((entt::entity)entity); }
	return valid;
}

void Scene::setInactive(Entity entity)
{
	this->setComponent<Inactive>(entity);
	if (this->hasComponents<AudioSource>(entity))
	{
		AudioSource& source = this->getComponent<AudioSource>(entity);
		if (source.playingb4Inactive = source.isPlaying())
		{
			source.pause();
		}
	}
}

void Scene::setActive(Entity entity)
{
	this->removeComponent<Inactive>(entity);
	if (this->hasComponents<AudioSource>(entity))
	{
		AudioSource& source = this->getComponent<AudioSource>(entity);
		if (source.playingb4Inactive)
		{
			source.play();
		}
	}
}

bool Scene::isActive(Entity entity)
{
	return !this->hasComponents<Inactive>(entity);
}

//void Scene::setAnimation(Entity entity, const std::string& animationName, bool resetTimer)
//{
//	if (this->hasComponents<MeshComponent, AnimationComponent>(entity))
//	{
//		AnimationComponent& aniComp = this->getComponent<AnimationComponent>(entity);
//		aniComp.timer *= !resetTimer;
//	}
//}

void Scene::setAnimation(Entity entity, const std::string& animationName, const std::string& slotName)
{
	if (this->hasComponents<MeshComponent, AnimationComponent>(entity))
	{
		const Mesh& mesh = this->sceneHandler->getResourceManager()->
			getMesh(this->getComponent<MeshComponent>(entity).meshID);
		const uint32_t aniIndex = mesh.getAnimationIndex(animationName);

		AnimationComponent& aniComp = this->getComponent<AnimationComponent>(entity);
		if (slotName == "")
		{
			for (int i = 0; i < NUM_MAX_ANIMATION_SLOTS; i++)
			{
				aniComp.aniSlots[i].animationIndex = aniIndex;
				aniComp.aniSlots[i].timer = 0.f;
				aniComp.aniSlots[i].timeScale = 1.f;
			}
		}
		else
		{
			const uint32_t slotIndex = mesh.getAnimationSlotIndex(slotName);
			aniComp.aniSlots[slotIndex].animationIndex = aniIndex;
			aniComp.aniSlots[slotIndex].timer = 0.f;
		}

	}
}

void Scene::init()
{

}

void Scene::start()
{

}

void Scene::update()
{

}

void Scene::onCollisionEnter(Entity e1, Entity e2)
{

}

void Scene::onCollisionStay(Entity e1, Entity e2)
{

}

void Scene::onCollisionExit(Entity e1, Entity e2)
{

}

void Scene::onTriggerEnter(Entity e1, Entity e2)
{

}

void Scene::onTriggerStay(Entity e1, Entity e2)
{

}

void Scene::onTriggerExit(Entity e1, Entity e2)
{

}

void Scene::setSceneHandler(SceneHandler& sceneHandler)
{
	this->sceneHandler = &sceneHandler;
}
