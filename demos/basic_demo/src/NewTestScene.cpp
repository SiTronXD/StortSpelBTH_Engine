#include <iostream>
#include "NewTestScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"
#include "vengine/test/TestScene2.hpp"

NewTestScene::NewTestScene()
{
}

NewTestScene::~NewTestScene()
{
}
void NewTestScene::init()
{	

}

void NewTestScene::start()
{
#if AUDIO
	uint32_t audioId = this->getResourceManager()->addSound("assets/sounds/test-audio.wav");

	audioSource1 = this->createEntity();
	this->setComponent<AudioSource>(audioSource1, audioId);
	this->setComponent<MeshComponent>(audioSource1, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->getComponent<Transform>(audioSource1).position.x = 30.f;
	volume1 = this->getComponent<AudioSource>(audioSource1).getVolume();

	audioSource2 = this->createEntity();
	this->setComponent<AudioSource>(audioSource2, audioId);
	this->setComponent<MeshComponent>(audioSource2, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->getComponent<Transform>(audioSource2).position.x = -30.f;
	volume2 = this->getComponent<AudioSource>(audioSource2).getVolume();

	this->getAudioHandler()->setMasterVolume(0.5f);
	master = this->getAudioHandler()->getMasterVolume();
	this->getAudioHandler()->setMusic("assets/sounds/test-music.wav");
	this->getAudioHandler()->playMusic();

	this->getAudioHandler()->setMusicVolume(1.f);
	music = this->getAudioHandler()->getMusicVolume();
#endif
}

void NewTestScene::update()
{
#if AUDIO
	if (ImGui::Begin("Sound"))
	{
		ImGui::PushItemWidth(-100.f);

		if (this->hasComponents<AudioSource>(this->audioSource1))
		{
			AudioSource& source = this->getComponent<AudioSource>(this->audioSource1);
			static bool loop = false;

			if (ImGui::Button("Play 1"))
			{
				source.play();
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Loop", &loop))
			{
				source.setLooping(loop);
			}
			if (ImGui::DragFloat("Source 1 volume", &volume1, 0.01f, 0.f, 1.f))
			{
				source.setVolume(volume1);
			}
		}
		if (this->hasComponents<AudioSource>(this->audioSource2))
		{
			AudioSource& source = this->getComponent<AudioSource>(this->audioSource2);
			static bool loop = false;

			if (ImGui::Button("Play 2"))
			{
				source.play();
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Loop 2", &loop))
			{
				source.setLooping(loop);
			}
			if (ImGui::DragFloat("Source 2 volume", &volume2, 0.01f, 0.f, 1.f))
			{
				source.setVolume(volume2);
			}
		}
		
		if (ImGui::DragFloat("Music volume", &music, 0.01f, 0.f, 1.f))
		{
			this->getAudioHandler()->setMusicVolume(music);
		}
		if (ImGui::DragFloat("Master volume", &master, 0.01f, 0.f, 1.f))
		{
			this->getAudioHandler()->setMasterVolume(master);
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
#endif
}

void NewTestScene::onCollisionEnter(Entity e1, Entity e2)
{
	
}

void NewTestScene::onCollisionStay(Entity e1, Entity e2)
{
	
}

void NewTestScene::onCollisionExit(Entity e1, Entity e2)
{
	
}

void NewTestScene::onTriggerEnter(Entity e1, Entity e2)
{

}

void NewTestScene::onTriggerStay(Entity e1, Entity e2)
{

}

void NewTestScene::onTriggerExit(Entity e1, Entity e2)
{

}