#include "SceneTest.h"

static bool showDebug = false;
static bool showRay = false;

static bool trigger = false;
static glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);
static int colliderType = 0;
static float radius = 3.0f;
static float height = 9.0f;
static glm::vec3 extents = glm::vec3(2.0f);

static float mass = 1.0f;
static float gravityMultiplier = 1.0f;
static glm::vec3 rotFactor = glm::vec3(1.0f);
static glm::vec3 posFactor = glm::vec3(1.0f);

static const char* types[]{ "Sphere", "Box", "Capsule" };

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
			this->getComponent<Transform>(e).position = glm::vec3(x * 15, 30.0f, y * 15);
			this->getComponent<Transform>(e).rotation = SMath::getRandomVector(180.0f);
			this->setComponent<MeshComponent>(e, swarm);
			this->setComponent<Collider>(e, trigger, offset, (ColType)colliderType, radius, height, extents);
			this->setComponent<Rigidbody>(e, mass, gravityMultiplier, 0.0f, posFactor, rotFactor);

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

	ImGui::Begin("Physics settings");
	ImGui::Checkbox("Show Debug Shapes", &showDebug);
	ImGui::Checkbox("Show Ray", &showRay);
	ImGui::Text("Collider Parameters");
	ImGui::Checkbox("Trigger", &trigger);
	ImGui::DragFloat3("Offset", &offset[0], 0.25f);
	ImGui::ListBox("Type", &colliderType, types, 3);
	ImGui::DragFloat("Radius", &radius, 0.25f);
	ImGui::DragFloat("Height", &height, 0.25f);
	ImGui::DragFloat3("Extents", &extents[0], 0.25f);
	ImGui::Text("Rigidbody Parameters");
	ImGui::DragFloat("Mass", &mass, 0.25f);
	ImGui::DragFloat("Gravity Multiplier", &gravityMultiplier, 0.25f);
	ImGui::DragFloat3("Rot Factor", &rotFactor[0], 0.1f, 0.0f, 1.0f);
	ImGui::DragFloat3("Pos Factor", &posFactor[0], 0.1f, 0.0f, 1.0f);
	if (ImGui::Button("Restart simulation"))
	{
		for (int x = 0; x < NUM; x++)
		{
			for (int y = 0; y < NUM; y++)
			{
				Entity e = this->entities[y * NUM + x];
				this->getComponent<Transform>(e).position = glm::vec3(x * 15, 30.0f, y * 15);
				this->getComponent<Transform>(e).rotation = SMath::getRandomVector(180.0f);
				this->setComponent<Collider>(e, trigger, offset, (ColType)colliderType, radius, height, extents);
				this->setComponent<Rigidbody>(e, mass, gravityMultiplier, 0.0f, posFactor, rotFactor);
			}
		}
	}
	ImGui::End();

	this->getPhysicsEngine()->renderDebugShapes(showDebug);

	if (showRay)
	{
		Transform& camT = this->getComponent<Transform>(this->getMainCameraID());
		glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
		Ray ray{ camT.position, camT.forward() };
		RayPayload payload = this->getPhysicsEngine()->raycast(ray);

		this->getDebugRenderer()->renderLine(ray.pos - glm::vec3(0.0f, 0.5f, 0.0f), ray.pos + ray.dir * 100.0f, color);
		if (payload.hit)
		{
			Collider& col = this->getComponent<Collider>(payload.entity);
			Transform& transform = this->getComponent<Transform>(payload.entity);
			if (col.type == ColType::SPHERE)
			{
				this->getDebugRenderer()->renderSphere(transform.position + col.offset, col.radius, color);
			}
			else if (col.type == ColType::BOX)
			{
				this->getDebugRenderer()->renderBox(transform.position + col.offset, transform.rotation, col.extents * 2.0f, color);
			}
			else if (col.type == ColType::CAPSULE)
			{
				this->getDebugRenderer()->renderCapsule(transform.position + col.offset, transform.rotation, col.height, col.radius, color);
			}
			this->getDebugRenderer()->renderSphere(payload.hitPoint, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			this->getDebugRenderer()->renderLine(payload.hitPoint, payload.hitPoint + payload.hitNormal * 5.0f, glm::vec3(1.0f));
		}
	}
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