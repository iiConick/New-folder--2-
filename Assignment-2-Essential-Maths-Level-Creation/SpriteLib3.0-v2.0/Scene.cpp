#include "Scene.h"
#include "Utilities.h"

Scene::Scene(std::string name)
{
	m_physicsWorld = new b2World(m_gravity);
	m_name = name;
}

void Scene::Unload()
{
	if (m_sceneReg != nullptr)
	{
		delete m_sceneReg;
		m_sceneReg = nullptr;
	}

	if (m_physicsWorld != nullptr)
	{
		delete m_physicsWorld;
		m_physicsWorld = nullptr;
	}
}

void Scene::InitScene(float windowWidth, float windowHeight)
{
	//Default scene class init just includes a camera so stuff doesn't immediately crash
	printf("You shouldn't be running this, while initializing this Scene Type.\n");

	//Dynamically allocates the register
	if (m_sceneReg == nullptr)
	{
		m_sceneReg = new entt::registry;
	}

	//Attach the register
	ECS::AttachRegister(m_sceneReg);

	//Sets up aspect ratio for the camera
	float aspectRatio = windowWidth / windowHeight;

	//Setup Main Camera entity
	{
		//Creates Camera entity
		auto entity = ECS::CreateEntity();
		ECS::SetIsMainCamera(entity, true);

		//Creates new orthographic camera
		ECS::AttachComponent<Camera>(entity);
		//ECS::AttachComponent<HorizontalScroll>(entity);
		vec4 temp = vec4(-75.f, 75.f, -75.f, 75.f);
		ECS::GetComponent<Camera>(entity).SetOrthoSize(temp);
		ECS::GetComponent<Camera>(entity).SetWindowSize(vec2(float(windowWidth), float(windowHeight)));
		ECS::GetComponent<Camera>(entity).Orthographic(aspectRatio, temp.x, temp.y, temp.z, temp.w, -100.f, 100.f);
	}

	//Setup new Entity
	{
		//Creates entity
		auto entity = ECS::CreateEntity();
		m_helloWorldSign = entity;

		//Add components
		ECS::AttachComponent<Sprite>(entity);
		ECS::AttachComponent<Transform>(entity);

		//Sets up components
		std::string fileName = "HelloWorld.png";
		ECS::GetComponent<Sprite>(entity).LoadSprite(fileName, 100, 60);
		ECS::GetComponent<Sprite>(entity).SetTransparency(0.5f);
		ECS::GetComponent<Transform>(entity).SetPosition(vec3(0.f, 0.f, 0.f));
	}
}

void Scene::Update()
{
	auto& tempSpr = m_sceneReg->get<Sprite>(m_helloWorldSign);
	
	tempSpr.SetTransparency((0.5 * sin(Timer::time * 3.f)) + 0.5f);
}

void Scene::AdjustScrollOffset()
{
	float maxSizeX = ECS::GetComponent<Camera>(MainEntities::MainCamera()).GetOrthoSize().y;
	float maxSizeY = ECS::GetComponent<Camera>(MainEntities::MainCamera()).GetOrthoSize().w;

	float playerHalfSize = ECS::GetComponent<Sprite>(MainEntities::MainPlayer()).GetWidth() / 2.f;

	ECS::GetComponent<HorizontalScroll>(MainEntities::MainCamera()).SetOffset((maxSizeX * BackEnd::GetAspectRatio()) - playerHalfSize);
	ECS::GetComponent<VerticalScroll>(MainEntities::MainCamera()).SetOffset(maxSizeY - playerHalfSize);
}

void Scene::CreateCameraEntity(bool mainCamera, float windowWidth, float windowHeight, vec4 temp, 
									float zNear, float zFar)
{
	float aspectRatio = windowWidth / windowHeight;
	//Setup main camera
	{
		//Creates Camera Entity
		auto entity = ECS::CreateEntity();
		ECS::SetIsMainCamera(entity, mainCamera);

		ECS::AttachComponent<Camera>(entity);
		ECS::AttachComponent<HorizontalScroll>(entity);
		ECS::AttachComponent<VerticalScroll>(entity);

		//vec4 temp = vec4(left, right, bottom, top);
		ECS::GetComponent<Camera>(entity).SetOrthoSize(temp);
		ECS::GetComponent<Camera>(entity).SetWindowSize(vec2(float(windowWidth), float(windowHeight)));
		ECS::GetComponent<Camera>(entity).Orthographic(aspectRatio, temp.x, temp.y, temp.z, temp.w, zNear, zFar);

		ECS::GetComponent<HorizontalScroll>(entity).SetCam(&ECS::GetComponent<Camera>(entity));
		ECS::GetComponent<VerticalScroll>(entity).SetCam(&ECS::GetComponent<Camera>(entity));

	}
}

void Scene::CreateEntity(std::string fileName, int width, int height, float transparency, vec3 position)
{
	//Creates entity
	
	auto entity = ECS::CreateEntity();
	MainEntities::SetFocus(entity);
	ECS::AttachComponent<Sprite>(entity);
	ECS::AttachComponent<Transform>(entity);

	//Set up the components
	ECS::GetComponent<Sprite>(entity).LoadSprite(fileName, width, height);
	ECS::GetComponent<Sprite>(entity).SetTransparency(transparency);
	ECS::GetComponent<Transform>(entity).SetPosition(position);
}

void Scene::CreateMainEntity(b2World* m_physicsWorld,std::string fileName,int width,int height,float transparency,vec3 position,float shrink) {
	auto entity = ECS::CreateEntity();
	ECS::SetIsMainPlayer(entity, true);

	//Add components
	ECS::AttachComponent<Sprite>(entity);
	ECS::AttachComponent<Transform>(entity);
	ECS::AttachComponent<PhysicsBody>(entity);
	ECS::AttachComponent<CanJump>(entity);

	//Sets up the components
	ECS::GetComponent<Sprite>(entity).LoadSprite(fileName, width, height);
	ECS::GetComponent<Sprite>(entity).SetTransparency(transparency);
	ECS::GetComponent<Transform>(entity).SetPosition(position);

	auto& tempSpr = ECS::GetComponent<Sprite>(entity);
	auto& tempPhsBody = ECS::GetComponent<PhysicsBody>(entity);

	b2Body* tempBody;
	b2BodyDef tempDef;
	tempDef.type = b2_dynamicBody;
	tempDef.position.Set(position.x, position.y);

	tempBody = m_physicsWorld->CreateBody(&tempDef);

	tempPhsBody = PhysicsBody(entity, tempBody, float((tempSpr.GetHeight() - shrink) / 2.f), vec2(0.f, 0.f), false, PLAYER, ENEMY | OBJECTS | PICKUP | TRIGGER, 0.5f, 3.f);

	tempPhsBody.SetRotationAngleDeg(0.f);
	tempPhsBody.SetFixedRotation(true);
	tempPhsBody.SetColor(vec4(1.f, 0.f, 1.f, 0.3f));
	tempPhsBody.SetGravityScale(0.f);
}
void Scene::CreateStaticEntity(b2World* m_physicsWorld, std::string fileName,int width,int height,vec3 position,float shrinkX,
								float shrinkY,float tempDefPositionX, float tempDefPositionY,float rotation) 
{
	//Creates entity
	auto entity = ECS::CreateEntity();

	//Add components
	ECS::AttachComponent<Sprite>(entity);
	ECS::AttachComponent<Transform>(entity);
	ECS::AttachComponent<PhysicsBody>(entity);

	//Sets up components
	ECS::GetComponent<Sprite>(entity).LoadSprite(fileName, width, height);
	ECS::GetComponent<Transform>(entity).SetPosition(position);

	auto& tempSpr = ECS::GetComponent<Sprite>(entity);
	auto& tempPhsBody = ECS::GetComponent<PhysicsBody>(entity);

	b2Body* tempBody;
	b2BodyDef tempDef;
	tempDef.type = b2_staticBody;
	tempDef.position.Set(tempDefPositionX, tempDefPositionY);

	tempBody = m_physicsWorld->CreateBody(&tempDef);

	tempPhsBody = PhysicsBody(entity, tempBody, float(tempSpr.GetWidth() - shrinkX),
		float(tempSpr.GetHeight() - shrinkY), vec2(0.f, 0.f), false, GROUND, PLAYER | ENEMY);
	tempPhsBody.SetColor(vec4(0.f, 1.f, 0.f, 0.3f));
	if (rotation != 0.f)
	{
		tempPhsBody.SetRotationAngleDeg(rotation);
	}
}
void Scene::CreateMoveableEntity(b2World* m_physicsWorld, std::string fileName, int width, int height,float transparency, vec3 position, float shrinkX, 
								float shrinkY, float tempDefPositionX, float tempDefPositionY) {
	auto entity = ECS::CreateEntity();
	//Add components
	ECS::AttachComponent<Sprite>(entity);
	ECS::AttachComponent<Transform>(entity);
	ECS::AttachComponent<PhysicsBody>(entity);

	//Sets up the components
	ECS::GetComponent<Sprite>(entity).LoadSprite(fileName, width, height);
	ECS::GetComponent<Sprite>(entity).SetTransparency(transparency);
	ECS::GetComponent<Transform>(entity).SetPosition(position);

	auto& tempSpr = ECS::GetComponent<Sprite>(entity);
	auto& tempPhsBody = ECS::GetComponent<PhysicsBody>(entity);

	b2Body* tempBody;
	b2BodyDef tempDef;
	tempDef.type = b2_dynamicBody;
	tempDef.position.Set(tempDefPositionX, tempDefPositionY);

	tempBody = m_physicsWorld->CreateBody(&tempDef);

	//tempPhsBody = PhysicsBody(tempBody, float(tempSpr.GetWidth() - shrinkX), float(tempSpr.GetHeight() - shrinkY), vec2(0.f, 0.f), false);
	tempPhsBody = PhysicsBody(entity, tempBody, float((tempSpr.GetWidth() - shrinkY) / 2.f), vec2(0.f, 0.f), false, OBJECTS, GROUND | ENVIRONMENT | PLAYER | TRIGGER, 0.3f);

	tempPhsBody.SetColor(vec4(1.f, 0.f, 1.f, 0.3f));
	tempPhsBody.SetFixedRotation(true);
}
void Scene::CreateEnemy(int entity,b2World* m_physicsWorld,std::string fileName,int width,int height,vec3 position,float shrinkX,
								float shrinkY,float tempDefPositionX, float tempDefPositionY) 
{

	//Add components
	ECS::AttachComponent<Sprite>(entity);
	ECS::AttachComponent<Transform>(entity);
	ECS::AttachComponent<PhysicsBody>(entity);

	ECS::GetComponent<Sprite>(entity).LoadSprite(fileName, width, height);
	ECS::GetComponent<Transform>(entity).SetPosition(position);

	auto& tempSpr = ECS::GetComponent<Sprite>(entity);
	auto& tempPhsBody = ECS::GetComponent<PhysicsBody>(entity);

	b2Body* tempBody;
	b2BodyDef tempDef;
	tempDef.type = b2_staticBody;
	tempDef.position.Set(tempDefPositionX, tempDefPositionY);

	tempBody = m_physicsWorld->CreateBody(&tempDef);

	tempPhsBody = PhysicsBody(entity, tempBody, float(tempSpr.GetWidth() - shrinkX), float(tempSpr.GetHeight() - shrinkY), vec2(0.f, 0.f), false, ENVIRONMENT, PLAYER | ENEMY);
	tempPhsBody.SetColor(vec4(0.f, 1.f, 0.f, 0.3f));
}
void Scene::CreateTrigger(b2World* m_physicsWorld, std::string fileName,int target,float shrinkX,float shrinkY,vec3 position,float tempDefPositionX, float tempDefPositionY) {
	//Setup trigger
	
		//Creates entity
		auto entity = ECS::CreateEntity();

		//Add components
		ECS::AttachComponent<Transform>(entity);
		ECS::AttachComponent<PhysicsBody>(entity);
		ECS::AttachComponent<Trigger*>(entity);

		//Sets up components
		ECS::GetComponent<Transform>(entity).SetPosition(position);
		ECS::GetComponent<Trigger*>(entity) = new DestroyTrigger();
		ECS::GetComponent<Trigger*>(entity)->SetTriggerEntity(entity);
		ECS::GetComponent<Trigger*>(entity)->AddTargetEntity(target);

		auto& tempPhsBody = ECS::GetComponent<PhysicsBody>(entity);

		b2Body* tempBody;
		b2BodyDef tempDef;
		tempDef.type = b2_staticBody;
		tempDef.position.Set(tempDefPositionX, tempDefPositionY);

		tempBody = m_physicsWorld->CreateBody(&tempDef);

		tempPhsBody = PhysicsBody(entity, tempBody, float(40.f - shrinkX), float(40.f - shrinkY), vec2(0.f, 0.f), true, TRIGGER, PLAYER | OBJECTS);
		tempPhsBody.SetColor(vec4(1.f, 0.f, 0.f, 0.3f));
	


}

entt::registry* Scene::GetScene() const
{
	return m_sceneReg;
}

void Scene::SetScene(entt::registry& scene)
{
	m_sceneReg = &scene;
}

vec4 Scene::GetClearColor() const
{
	return m_clearColor;
}

void Scene::SetClearColor(vec4 clearColor)
{
	m_clearColor = clearColor;
}

std::string Scene::GetName() const
{
	return m_name;
}

void Scene::SetName(std::string name)
{
	m_name = name;
}

b2Vec2 Scene::GetGravity() const
{
	return m_gravity;
}

void Scene::SetGravity(b2Vec2 grav)
{
	m_gravity = grav;
}

b2World & Scene::GetPhysicsWorld()
{
	return *m_physicsWorld;
}

void Scene::SetWindowSize(float windowWidth, float windowHeight)
{
	//TODO: Find new way to get the main camera
	auto& tempCam = m_sceneReg->get<Camera>(MainEntities::MainCamera());
	
	tempCam.SetWindowSize(vec2(windowWidth, windowHeight));
	tempCam.Orthographic(float(windowWidth / windowHeight), tempCam.GetOrthoSize().x, tempCam.GetOrthoSize().y,
															tempCam.GetOrthoSize().z, tempCam.GetOrthoSize().w,
															tempCam.GetNear(), tempCam.GetFar());
}
