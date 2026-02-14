#include "PhysicsManager.h"

float PhysicsManager::pixels_per_meter = 16.f;
float PhysicsManager::meters_per_pixel = 1.0f / pixels_per_meter;

int PhysicsManager::sub_step_count;
SFMLDebugDrawContext PhysicsManager::ctx;
b2DebugDraw PhysicsManager::debug_draw;
b2WorldId PhysicsManager::world_id;

void PhysicsManager::init(
	float gravity, float P2M, float M2P, 
	int sub_step_count
)
{
	PhysicsManager::pixels_per_meter = P2M;
	PhysicsManager::meters_per_pixel = M2P;
	PhysicsManager::sub_step_count = sub_step_count;

	b2Vec2 gravity_vec = { 0.f, gravity };
	b2WorldDef world_def = b2DefaultWorldDef();
	world_def.gravity = gravity_vec;

	/*world_def.contactHertz = 60.0f;
	world_def.contactDampingRatio = 1.0f; 
	world_def.maxContactPushSpeed = 20.0f;
	world_def.enableContinuous = true;   
	world_def.maximumLinearSpeed = 50.0f;*/

	world_id = b2CreateWorld(&world_def);

	ctx = { NULL, 0 };
}

void PhysicsManager::init_debug_draw(sf::RenderWindow& window)
{
	ctx.target = &window;
	ctx.pixelsPerMeter = pixels_per_meter;
	SetupB2DebugDraw_SFML(debug_draw, ctx);
}

void PhysicsManager::run(float dt)
{
	b2World_Step(world_id, dt, sub_step_count);
	auto events = b2World_GetContactEvents(world_id);
}

void PhysicsManager::draw()
{
	if (!ctx.target) {
		return;
	}
	b2World_Draw(world_id, &debug_draw);
}

void PhysicsManager::destroy()
{
	b2DestroyWorld(world_id);
}
