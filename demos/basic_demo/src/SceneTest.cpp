#include "SceneTest.h"

SceneTest::SceneTest()
{

}

SceneTest::~SceneTest()
{

}

void SceneTest::init()
{
	Entity cam = this->createEntity();
	this->setComponent<Camera>(cam);
	this->setMainCamera(cam);
	this->getComponent<Transform>(cam).position = glm::vec3(NUM / 2 * 10, 25.0f, -25.0f);

	this->setFogAbsorption(0.0f);

	// Directional light
	Entity directionalLightEntity = this->createEntity();
	this->setComponent<DirectionalLight>(directionalLightEntity);
	this->getComponent<DirectionalLight>(directionalLightEntity)
		.color = glm::vec3(0.5);
	this->getComponent<DirectionalLight>(directionalLightEntity)
		.direction = glm::vec3(-1.0f, -1.0f, 1.0f);
	this->getComponent<DirectionalLight>(directionalLightEntity)
		.cascadeDepthScale = 5.0f;

	this->floor = this->createEntity();
	this->getComponent<Transform>(this->floor).scale = glm::vec3(1000.0f, 1.0f, 1000.0f);
	this->setComponent<MeshComponent>(this->floor, 0);
	this->setComponent<Collider>(this->floor, Collider::createBox(glm::vec3(1000.0f, 1.0f, 1000.0f)));

	int swarm = this->getResourceManager()->addMesh("assets/models/Swarm_Model.fbx", "assets/textures/swarmTextures/");

	for (int x = 0; x < NUM; x++)
	{
		for (int y = 0; y < NUM; y++)
		{
			Entity e = this->createEntity();
			this->getComponent<Transform>(e).position = glm::vec3(x * 15, 50.0f, y * 15);
			this->getComponent<Transform>(e).rotation = SMath::getRandomVector(180.0f);
			this->setComponent<MeshComponent>(e, swarm);
			this->setComponent<Collider>(e, Collider::createBox(glm::vec3(4.0f), glm::vec3(0.0f, -2.0f, 0.0f)));
			this->setComponent<Rigidbody>(e);

			this->entities[y * NUM + x] = e;
		}
	}
}

void SceneTest::start()
{

}

void SceneTest::update()
{
	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), Input::isKeyDown(Keys::Q) - Input::isKeyDown(Keys::E), Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		glm::vec3 rotVec = glm::vec3(Input::isKeyDown(Keys::I) - Input::isKeyDown(Keys::K), Input::isKeyDown(Keys::J) - Input::isKeyDown(Keys::L), 0.0f);
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += (moveVec.x * camTransform.right() + glm::vec3(0.0f, moveVec.y, 0.0f) + moveVec.z * camTransform.forward()) * 25.0f * Time::getDT();
		camTransform.rotation += rotVec * 100.0f * Time::getDT();
	}

	static bool showDebug = false;
	static bool showRay = false;

	static bool trigger = false;
	static glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);
	static int colliderType = 0;
	static float radius = 1.0f;
	static float height = 1.0f;
	static glm::vec3 extents = glm::vec3(1.0f);

	static float mass = 1.0f;
	static float gravityMultiplier = 1.0f;
	static glm::vec3 rotFactor = glm::vec3(1.0f);
	static glm::vec3 posFactor = glm::vec3(1.0f);

	static const char* types[]{ "Sphere", "Box", "Capsule" };

	ImGui::Begin("Physics settings");
	ImGui::Checkbox("Show Debug Shapes", &showDebug);
	ImGui::Checkbox("Show Ray", &showDebug);
	ImGui::Text("Collider Parameters");
	ImGui::Checkbox("Trigger", &showDebug);
	ImGui::DragFloat3("Offset", &offset[0]);
	ImGui::ListBox("Type", &colliderType, types, 3);
	ImGui::DragFloat("Radius", &radius);
	ImGui::DragFloat("Height", &height);
	ImGui::DragFloat3("Extents", &extents[0]);
	ImGui::Text("Rigidbody Parameters");
	if (ImGui::Button("Restart simulation"))
	{
		for (int x = 0; x < NUM; x++)
		{
			for (int y = 0; y < NUM; y++)
			{
				Entity e = this->entities[y * NUM + x];
				this->getComponent<Transform>(e).position = glm::vec3(x * 15, 50.0f, y * 15);
				this->getComponent<Transform>(e).rotation = SMath::getRandomVector(180.0f);
				this->getComponent<Rigidbody>(e).assigned = false;
				this->getComponent<Rigidbody>(e).velocity = glm::vec3(0.0f);
			}
		}
	}
	ImGui::End();

	this->getPhysicsEngine()->renderDebugShapes(showDebug);
}

void SceneTest::onCollisionEnter(Entity e1, Entity e2)
{

}
void SceneTest::onCollisionStay(Entity e1, Entity e2)
{

}
void SceneTest::onCollisionExit(Entity e1, Entity e2)
{

}

void SceneTest::onTriggerEnter(Entity e1, Entity e2)
{
					 
}
void SceneTest::onTriggerStay(Entity e1, Entity e2)
{
					 
}
void SceneTest::onTriggerExit(Entity e1, Entity e2)
{

}