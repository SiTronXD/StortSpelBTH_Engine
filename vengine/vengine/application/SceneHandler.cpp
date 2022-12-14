#include "pch.h"
#include "SceneHandler.hpp"
#include "Time.hpp"
#include "../components/AnimationComponent.hpp"
#include "../graphics/VulkanRenderer.hpp"
#include "../graphics/UIRenderer.hpp"
#include "../lua/ScriptHandler.h"
#include "../network/ServerEngine/NetworkScene.h"

void SceneHandler::initSubsystems()
{
	if (physicsEngine != nullptr)
	{
		this->physicsEngine->init();
	}
	
	if (this->audioHandler)
	{
		// Reset all audio sources
		this->audioHandler->reset();
	}

	// Init scene
	this->scene->init();
	if (this->luaScript.size() != 0)
	{
		this->scriptHandler->runScript(this->luaScript);
	}
	this->scene->start();

	// Init renderer for scene
	if (vulkanRenderer != nullptr)
	{
		this->vulkanRenderer->initForScene(this->scene);
	}

	// Reset delta time counter
	Time::reset();
}

void SceneHandler::updatePreScene()
{
	// Update animation timers
	auto animView = this->scene->getSceneReg().view<AnimationComponent, MeshComponent>(entt::exclude<Inactive>);
	animView.each([&]
	(AnimationComponent& animationComponent, const MeshComponent& meshComponent)
		{
			Mesh& currentMesh = this->resourceManager->getMesh(meshComponent.meshID);
			const MeshData& meshData = currentMesh.getMeshData();

			for (uint32_t i = 0; i < NUM_MAX_ANIMATION_SLOTS; i++)
			{
				AnimationSlot& aniSlot = animationComponent.aniSlots[i];
				aniSlot.timer += Time::getDT() * 24.0f * aniSlot.timeScale;
				if (aniSlot.timer >= meshData.animations[aniSlot.animationIndex].endTime)
				{
					aniSlot.timer -= meshData.animations[aniSlot.animationIndex].endTime;
					aniSlot.finishedCycle = true;
				}

				if (aniSlot.nAnimationIndex != ~0u)
				{
					const Animation& nAnimation = meshData.animations[aniSlot.nAnimationIndex];
					// Still in transition
					if (aniSlot.alpha < 1.f)
					{
						aniSlot.nTimer += Time::getDT() * 24.f * aniSlot.nTimeScale;
						if (aniSlot.nTimer >= nAnimation.endTime)
						{
							aniSlot.nTimer -= nAnimation.endTime;
						}

						// Clamping alpha makes sure mesh doesn't "blink" when transitionTime == 0.f
						// glm::mix is defined beyond [0, 1] causing incorret results when alpha > 1.f || alpha < 0.f
						aniSlot.alpha += (1.f / aniSlot.transitionTime) * Time::getDT();
						if (aniSlot.alpha > 1.f)
						{
							aniSlot.alpha = 1.f;
						}
					}
					else // Switch and reset 
					{
						aniSlot.alpha = 0.f;

						aniSlot.animationIndex = aniSlot.nAnimationIndex;
						aniSlot.timer = aniSlot.nTimer;
						aniSlot.timeScale = aniSlot.nTimeScale;

						aniSlot.nAnimationIndex = ~0u;
						aniSlot.nTimer = 0.f;
					}
				}
			}

			// Store bone transformations
			currentMesh.getBoneTransforms(animationComponent);
		}
	);
}

SceneHandler::SceneHandler()
	: scene(nullptr), 
	nextScene(nullptr), 
	networkHandler(nullptr), 
	scriptHandler(nullptr), 
	aiHandler(nullptr),
	resourceManager(nullptr),
    physicsEngine(nullptr),
	vulkanRenderer(nullptr),
	uiRenderer(nullptr), 
	debugRenderer(nullptr),
	window(nullptr),
	audioHandler(nullptr)
{ }

SceneHandler::~SceneHandler()
{
	delete this->scene;
}

void SceneHandler::update()
{
	this->updatePreScene();

	this->scene->updateSystems();
	this->scriptHandler->updateSystems(this->scene->getLuaSystems());
	this->scene->update();
}

void SceneHandler::updateToNextScene()
{
	// Make sure a scene can be switched to
	if (this->nextScene != nullptr)
	{
        NetworkScene* nScene = dynamic_cast<NetworkScene*>(scene);
        if (nScene != nullptr)
        {
            ((NetworkScene*)nextScene)->setServer((nScene)->getServer());
        }
		// Delete old scene
		delete this->scene;
		this->scene = nullptr;

		// Switch
		this->scene = this->nextScene;
		this->nextScene = nullptr;
		this->luaScript = this->nextLuaScript;

		// Init 
		this->initSubsystems();
	}
}

void SceneHandler::prepareForRendering()
{
	// Update transform matrices
	auto view = this->scene->getSceneReg().view<Transform>(entt::exclude<Inactive>);
	auto func = [](Transform& transform)
	{
		transform.updateMatrix();
	};
	view.each(func);

	ScriptHandler& scriptHandler = *this->getScriptHandler();
	auto uiView = this->scene->getSceneReg().view<UIArea, Script>(entt::exclude<Inactive>);
	auto uiFunc = [&](const auto& entity, UIArea& uiArea, Script& script)
	{
		if (uiArea.isHovering())
		{
			scriptHandler.runFunction((Entity)entity, script, "onHover");
			if (uiArea.isClicking())
			{
				scriptHandler.runFunction((Entity)entity, script, "onClick");
			}
		}
	};
	uiView.each(uiFunc);
}

void SceneHandler::setScene(Scene* scene, std::string path)
{
	if (this->nextScene == nullptr)
	{
		this->nextScene = scene;
		this->nextScene->setSceneHandler(*this);
		this->nextLuaScript = path;
	}
	// Gameplay programmer tried to switch scene multiple times during 1 frame.
	else
	{
		// Deallocate unused scene
		delete scene;
	}
}

void SceneHandler::setNetworkHandler(NetworkHandler* networkHandler)
{
	this->networkHandler = networkHandler;
}

void SceneHandler::setScriptHandler(ScriptHandler* scriptHandler)
{
	this->scriptHandler = scriptHandler;
}

ScriptHandler* SceneHandler::getScriptHandler()
{
	return this->scriptHandler;
}

void SceneHandler::setAIHandler(AIHandler* aiHandler)
{
    this->aiHandler = aiHandler;
}

void SceneHandler::setPhysicsEngine(PhysicsEngine* physicsEngine)
{
	this->physicsEngine = physicsEngine;
}

void SceneHandler::setResourceManager(ResourceManager* resourceManager)
{
	this->resourceManager = resourceManager;
}

void SceneHandler::setUIRenderer(UIRenderer* uiRenderer)
{
	this->uiRenderer = uiRenderer;
}

UIRenderer* SceneHandler::getUIRenderer()
{
	return this->uiRenderer;
}

void SceneHandler::setDebugRenderer(DebugRenderer* debugRenderer)
{
	this->debugRenderer = debugRenderer;
}

DebugRenderer* SceneHandler::getDebugRenderer()
{
	return this->debugRenderer;
}

void SceneHandler::setVulkanRenderer(VulkanRenderer* vulkanRenderer)
{
	this->vulkanRenderer = vulkanRenderer;
}

VulkanRenderer* SceneHandler::getVulkanRenderer()
{
	return this->vulkanRenderer;
}

Scene* SceneHandler::getScene() const
{
	return this->scene;
}

void SceneHandler::setWindow(Window* window)
{
	this->window = window;
}

Window* SceneHandler::getWindow()
{
	return this->window;
}

void SceneHandler::setAudioHandler(AudioHandler* audioHandler)
{
	this->audioHandler = audioHandler;
}

AudioHandler* SceneHandler::getAudioHandler()
{
	return this->audioHandler;
}
