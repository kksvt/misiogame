#include "PhysicsManager.h"
#include "SpriteComponent.h"
#include "EntityManager.h"
#include <iostream>

float PhysicsManager::pixels_per_meter = 16.f;
float PhysicsManager::meters_per_pixel = 1.0f / pixels_per_meter;

int PhysicsManager::sub_step_count;
SFMLDebugDrawContext PhysicsManager::ctx;
b2DebugDraw PhysicsManager::debug_draw;
b2WorldId PhysicsManager::world_id;
std::unordered_map<Entity*, sf::Vector2f> PhysicsManager::move;

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

	world_def.contactHertz = 60.0f;
	world_def.contactDampingRatio = 1.0f; 
	/*world_def.maxContactPushSpeed = 20.0f;
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

//passing total_time here is questionable, but its pretty late and im tired
void PhysicsManager::run(float dt, uint64_t total_time)
{
	for (auto &[ent, desired_movement] : move) {
		ent->get_physics()->move(desired_movement);
	}
	move.clear();
	b2World_Step(world_id, dt, sub_step_count);

	auto sensor_events = b2World_GetSensorEvents(world_id);

	for (int i = 0; i < sensor_events.beginCount; ++i) {
		auto event = sensor_events.beginEvents[i];

		if (!b2Shape_IsValid(event.sensorShapeId) || !b2Shape_IsValid(event.visitorShapeId)) {
			continue;
		}
		
		auto body_id = b2Shape_GetBody(event.sensorShapeId);
		auto self = static_cast<Entity*>(b2Body_GetUserData(body_id));

		auto other_body = b2Shape_GetBody(event.visitorShapeId);
		auto other = static_cast<Entity*>(b2Body_GetUserData(other_body));

		if (!self || !other) {
			continue;
		}

		self->sensor_touch(other);
	}

	for (int i = 0; i < sensor_events.endCount; ++i) {
		auto event = sensor_events.endEvents[i];

		if (!b2Shape_IsValid(event.sensorShapeId) || !b2Shape_IsValid(event.visitorShapeId)) {
			continue;
		}

		auto body_id = b2Shape_GetBody(event.sensorShapeId);
		auto self = static_cast<Entity*>(b2Body_GetUserData(body_id));

		auto other_body = b2Shape_GetBody(event.visitorShapeId);
		auto other = static_cast<Entity*>(b2Body_GetUserData(other_body));

		if (!self || !other) {
			continue;
		}

		self->sensor_stop_touch(other);
	}

	auto events = b2World_GetContactEvents(world_id);

	for (int i = 0; i < events.beginCount; ++i) {
		auto event = events.beginEvents[i];

		auto shape_a = event.shapeIdA;
		auto shape_b = event.shapeIdB;

		if (!b2Shape_IsValid(shape_a) || !b2Shape_IsValid(shape_b)) {
			continue;
		}

		auto body_a = b2Shape_GetBody(shape_a);
		auto body_b = b2Shape_GetBody(shape_b);

		auto ent_a = static_cast<Entity*>(b2Body_GetUserData(body_a));
		auto ent_b = static_cast<Entity*>(b2Body_GetUserData(body_b));

		if (!ent_a || !ent_b) {
			continue;
		}

		ent_a->touch(ent_b, total_time, event.manifold.points, event.manifold.pointCount, &event.manifold.normal);
		ent_b->touch(ent_a, total_time, event.manifold.points, event.manifold.pointCount, &event.manifold.normal);
	}

	for (int i = 0; i < events.endCount; ++i) {
		auto event = events.endEvents[i];

		auto shape_a = event.shapeIdA;
		auto shape_b = event.shapeIdB;

		if (!b2Shape_IsValid(shape_a) || !b2Shape_IsValid(shape_b)) {
			continue;
		}

		auto body_a = b2Shape_GetBody(shape_a);
		auto body_b = b2Shape_GetBody(shape_b);

		auto ent_a = static_cast<Entity*>(b2Body_GetUserData(body_a));
		auto ent_b = static_cast<Entity*>(b2Body_GetUserData(body_b));

		if (!ent_a || !ent_b) {
			continue;
		}

		ent_a->stop_touch(ent_b);
		ent_b->stop_touch(ent_a);
	}
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

static sf::Vector2f VectorIntToFloat(const sf::Vector2i& v) {
	return sf::Vector2f(static_cast<float>(v.x), static_cast<float>(v.y));
}

//not optimal, but its better than having each individual tile as a separate physics body
void PhysicsManager::create_physical_entities_for_tiles(const sf::Vector2u& render_size)
{
	std::vector<SpriteComponent_t*> sprites;
	sprites.reserve(EntityManager::get_num_allocated_entities());
	const float cmp_eps = 0.05f;
	for (uint32_t i = 0; i < EntityManager::get_num_allocated_entities(); ++i) {
		Entity* ent = EntityManager::get_entity(i);
		if (!ent || !ent->get_sprite() ||
			!(ent->get_serializable_type() >= SET_TILE_FIRST && ent->get_serializable_type() <= SET_TILE_LAST)) {
			continue;
		}
		sprites.push_back(ent->get_sprite());
	}

	std::sort(sprites.begin(), sprites.end(), [](SpriteComponent_t* a, SpriteComponent_t* b) {
		auto pos_a = a->get_position();
		auto pos_b = b->get_position();
		if (pos_a.y < pos_b.y) {
			return true;
		}
		if (pos_a.y == pos_b.y) {
			return pos_a.x < pos_b.x;
		}
		return false;
		});

	for (auto& it : sprites) {
		auto pos = it->get_position();
	}

	for (uint32_t i = 0; i < sprites.size(); ++i) {
		auto sprite = sprites[i];
		auto start = sprite->get_position();
		auto size = VectorIntToFloat(sprite->get_size());
		auto end = sprite->get_position() + size;
		for (uint32_t j = i + 1; j < sprites.size(); ++j) {
			auto new_sprite = sprites[j];
			auto pos = new_sprite->get_position();
			auto new_size = VectorIntToFloat(new_sprite->get_size());
			if (std::abs(size.x - new_size.x) > cmp_eps ||
				std::abs(size.y - new_size.y) > cmp_eps ||
				std::abs(end.x - pos.x) > cmp_eps ||
				std::abs(start.y - pos.y) > cmp_eps) {
				break;
			}
			end.x += new_size.x;
			i = j;
		}

		//std::cout << "Start of physics body: " << start.x << ", " << start.y << "\n";
		//std::cout << "End of physics body: " << end.x << ", " << end.y << "\n";
		EntityManager::create_physical_box(end.x - start.x, end.y - start.y, start.x, start.y);
	}

	//create map boundaries
	EntityManager::create_physical_box(0.1f, render_size.y, 0.f, 0.f);
	EntityManager::create_physical_box(0.1f, render_size.y, render_size.x, 0.f);
	EntityManager::create_physical_box(render_size.x, 0.1f, 0.f, 0.f);

	//create a trigger hurt
	EntityManager::create_trigger_hurt(render_size.x, 16.f, 0.f, render_size.y - 16.f);
}

void PhysicsManager::queue_movement(Entity* ent, const sf::Vector2f& desired_movement)
{
	move.insert({ ent, desired_movement });
}

void PhysicsManager::remove_movement(Entity* ent)
{
	auto it = move.find(ent);
	if (it != move.end()) {
		move.erase(it);
	}
}

void PhysicsManager::clear_queue()
{
	move.clear();
}
