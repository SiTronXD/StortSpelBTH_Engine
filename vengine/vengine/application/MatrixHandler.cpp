#include "MatrixHandler.hpp"
#include "SceneHandler.hpp"

MatrixHandler::MatrixHandler() 
	: sceneHandler(nullptr) 
{
}

void MatrixHandler::setSceneHandler(SceneHandler* sceneHandler) 
{
	this->sceneHandler = sceneHandler;
}

void MatrixHandler::update() 
{
	auto tView = this->sceneHandler->getScene()->getSceneReg().view<Transform>();

	tView.each([](Transform& transform)
	{
		transform.matrix = glm::translate(glm::mat4(1.0f), transform.position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), transform.scale);
	});
}