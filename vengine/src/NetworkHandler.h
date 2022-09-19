#pragma once
#include "Server.h"
#include "Client.h"
#include "Time.hpp"
#include "MeshComponent.hpp"

class NetworkHandler {
public:
	NetworkHandler();
	virtual ~NetworkHandler();
	void getSceneHandler(SceneHandler*& sceneHandler);
	
	//SERVER
	void createServer();//create new server on a new thread
	void deleteServer();
	//CLIENT
	Client* createClient(std::string name = "BOB");//should return a client
	Client* getClient();
	void connectClientToThis();
	void connectClient(std::string serverIP);
	void disconnectClient();

	void updateNetWork();
private:

	std::thread* serverThread;
	bool shutDownServer;
	Client* client;
	SceneHandler* sceneHandler;

	//HELPERS
	float fx, fy, fz, fa, fb, fc;
	int ix, iy, iz, ia, ib, ic;
	std::vector<int> otherPlayers;
	std::vector<int> monsters;

};