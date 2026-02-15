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

static sf::Vector2f VectorIntToFloat(const sf::Vector2i& v) {
	return sf::Vector2f(static_cast<float>(v.x), static_cast<float>(v.y));
}

//not optimal, but its better than having each individual tile as a separate physics body
void PhysicsManager::create_physical_entities_for_tiles()
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
		std::cout << "(" << pos.x << ", " << pos.y << ")\n";
	}

	for (uint32_t i = 0, j, k; i < sprites.size(); ++i) {
		auto sprite = sprites[i];
		auto start = sprite->get_position();
		auto size = VectorIntToFloat(sprite->get_size());
		auto end = sprite->get_position() + size;
		for (j = i + 1; j < sprites.size(); ++j) {
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
}
