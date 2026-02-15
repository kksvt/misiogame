#pragma once

#include "Entity.h"
#include "GrassTiles.h"
#include <vector>
#include <set>

class Level;

class EntityManager {
public:
	static Level* current_level;
	static int16_t num_lives;
	static int16_t score;
	static int16_t num_keys;

	static int create_empty_entity(const std::string& name, uint8_t type, uint8_t set_type);
	static Entity* create_player(const std::string& sprite_path, 
		int sprite_width, int sprite_height,
		bool enable_physics, float hitbox_width, float hitbox_height,
		float pos_x, float pos_y,
		const std::initializer_list<uint8_t>& num_frames,
		const std::initializer_list<uint8_t>& animation_speed);

	static void Player_Think(Entity* self, uint64_t time);
	static void Player_Touch(Entity* self, Entity* other, uint64_t total_time,
		b2ManifoldPoint points[2], const int num_points, b2Vec2* normal);
	static void Player_Stop_Touch(Entity* self, Entity* other);
	static void Player_Non_Tangible_Touch(Entity* self, Entity* other);
	static void Player_Non_Tangible_Stop_Touch(Entity* self, Entity* other);

	static Entity* create_concrete_player(bool enable_physics, float pos_x, float pos_y);

	static Entity* create_tile_unanimated(const std::string& sprite_path,
		int sprite_width, int sprite_height,
		float hitbox_width, float hitbox_height,
		float pos_x, float pos_y, uint8_t num_frames
	);

	static Entity* create_grass_tile(uint8_t grass_tile_type, float pos_x, float pos_y);

	static Entity* create_background(const std::string& sprite_path, int width, int height);

	static Entity* create_physical_box(float hitbox_width, float hitbox_height, float pos_x, float pos_y);
	static Entity* create_trigger_hurt(float hitbox_width, float hitbox_height, float pos_x, float pos_y);

	static Entity* create_baddie_one(bool enable_physics, float pos_x, float pos_y);
	static void Purple_Baddie_AI(Entity* self, uint64_t time);
	static void Purple_Baddie_Touch(Entity* self, Entity* other, uint64_t total_time,
		b2ManifoldPoint points[2], const int num_points, b2Vec2* normal);

	static Entity* create_door_locked(bool enable_physics, float pos_x, float pos_y);
	static Entity* create_door_exit(bool enable_physics, float pos_x, float pos_y);
	static Entity* create_key(bool enable_physics, float pos_x, float pos_y);
	static Entity* create_medpak(bool enable_physics, float pos_x, float pos_y);
	static Entity* create_info_sign(bool enable_physics, float pos_x, float pos_y);
	static Entity* create_spikes(bool enable_physics, float pos_x, float pos_y);
	static Entity* create_food(bool enable_physics, float pos_x, float pos_y);

	static void update(uint64_t time);
	static void queue_remove(int entity_num);
	static Entity* get_entity(int entity_num);
	static Entity* get_entity(float pos_x, float pos_y, int ignore);
	static void remove_marked();
	static void remove_all();
	static uint32_t get_num_active_entities();
	static uint32_t get_num_allocated_entities() { return all_entities_.size(); }
	static void draw(sf::RenderWindow& window);
private:
	EntityManager();
	static std::vector<EntityPtr_t> all_entities_;
	static std::set<int> remove_entities_;
};