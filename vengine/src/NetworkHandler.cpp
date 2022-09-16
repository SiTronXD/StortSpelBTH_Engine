#include "NetworkHandler.h"
#include "Timer.h"
#include <iostream>

int serverMain(bool &shutDownServer) {
	Timer serverTime;
	Server s;

	while (!shutDownServer) {
		s.update(serverTime.getDT());
		serverTime.updateDeltaTime();
	}
	return 1;
}

NetWorkHandler::NetWorkHandler()
{
	fx = fy = fz = fa = fb = fc = 0.f;
	ix = iy = iz = ia = ib = ic = 0;
	shutDownServer = false;
	client = nullptr;
	serverThread = nullptr;
}

NetWorkHandler::~NetWorkHandler()
{
	if (serverThread != nullptr) {
		serverThread->join();
		delete serverThread;
	}
}

void NetWorkHandler::getSceneHandler(SceneHandler*& sceneHandler)
{
	this->sceneHandler = sceneHandler;
}

void NetWorkHandler::createServer()
{
	serverThread = new std::thread(serverMain, std::ref(this->shutDownServer));
}

void NetWorkHandler::deleteServer()
{
	if (serverThread != nullptr) {
		shutDownServer = true;
		serverThread->join();
		delete serverThread;
	}
}

Client* NetWorkHandler::createClient(std::string name)
{
	if (client == nullptr) {
		client = new Client(name);
	}
	return client;
}

Client* NetWorkHandler::getClient()
{
	return client;
}

void NetWorkHandler::connectClient(std::string serverIP)
{
	if (client == nullptr) {
		std::cout << "client doesn't exist" << std::endl;
		return;
	}
	client->connect(serverIP);
}

void NetWorkHandler::updateNetWork()
{
	if (client == nullptr) {
		return;
	}
	client->update(Time::getDT());
	//tcp
	sf::Packet cTCPP = client->getTCPDataFromServer();
	int gameEvent;
	while (!cTCPP.endOfPacket()) {
		cTCPP >> gameEvent;
		if (gameEvent == GameEvents::DISCONNECT) {
			this->disconnectClient();
		}
		else if (gameEvent == GameEvents::PlayerJoined) {
			otherPlayers.push_back(sceneHandler->getScene()->createEntity());
			sceneHandler->getScene()->setComponent<MeshComponent>(otherPlayers[otherPlayers.size() - 1]);
			//sceneHandler->getScene()->setComponent<name>(players[players.size() - 1]);
		}
		else if (gameEvent == GameEvents::SpawnEnemy) {
			//ix = what type of enemy
			cTCPP >> ix;
			//should we really create a new entity everytime?
			iy = sceneHandler->getScene()->createEntity();
			monsters.push_back(iy);

			sceneHandler->getScene()->setComponent<MeshComponent>(iy);
			
			cTCPP >> fx >> fy >> fz;
			Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
			transform.position = glm::vec3(fx, fy, fz);
		}
		else if (gameEvent == GameEvents::SpawnEnemies) {
			//ix = what type of enemy
			cTCPP >> ix >> iy;

			for (int i = 0; i < iy; i++) {
				iz = sceneHandler->getScene()->createEntity();
				sceneHandler->getScene()->setComponent<MeshComponent>(iz);

				//ix = what type of enemy
				cTCPP >> fx >> fy >> fz;
				Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
				transform.position = glm::vec3(fx, fy, fz);
			}
		}
		else if (gameEvent == GameEvents::Explosion) {
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::MonsterDied) {
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::PlayerShoot) {
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::GAMEDATA) {
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::PlayerDied) {
			//don't know how this should be implemented right now
		}

	}
	sf::Packet cUDPP = client->getUDPDataFromServer();
	while (!cUDPP.endOfPacket()) {
		cUDPP >> gameEvent;
		if (gameEvent == GameEvents::UpdatePlayerPos) {
			//ix = amount of players
			cUDPP >> ix;
			for (int i = 0; i < ix; i++) {
				//fxyz position, fabc rotation
				cUDPP >> fx >> fy >> fz >> fa >> fb >> fc;
				Transform& transform = sceneHandler->getScene()->getComponent<Transform>(otherPlayers[i]);
				transform.position = glm::vec3(fx, fy, fz);
				transform.rotation = glm::vec3(fa, fb, fc);
			}
		}
		else if (gameEvent == GameEvents::UpdateMonsterPos) {
			//ix number of monsters
			cUDPP >> ix;
			for (int i = 0; i < ix; i++) {
				//fxyz position, fabc rotation
				cUDPP >> fx >> fy >> fz >> fa >> fb >> fc;
				Transform& transform = sceneHandler->getScene()->getComponent<Transform>(monsters[i]);
				transform.position = glm::vec3(fx, fy, fz);
				transform.rotation = glm::vec3(fa, fb, fc);
			}
		}
	}

	
		
}

void NetWorkHandler::disconnectClient()
{
	client->disconnect();
}


