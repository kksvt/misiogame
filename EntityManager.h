#pragma once

#include "Entity.h"
#include "GrassTiles.h"
#include <vector>
#include <set>

class EntityManager {
public:
	static int create_empty_entity(const std::string& name);
	static Entity* create_player(const std::string& sprite_path, 
		int sprite_width, int sprite_height,
		float hitbox_width, float hitbox_height, 
		float pos_x, float pos_y,
		const std::initializer_list<uint8_t>& num_frames,
		const std::initializer_list<uint8_t>& animation_speed);

	static Entity* create_tile_unanimated(const std::string& sprite_path,
		int sprite_width, int sprite_height,
		float hitbox_width, float hitbox_height,
		float pos_x, float pos_y, uint8_t num_frames
	);

	static Entity* create_grass_tile(uint8_t grass_tile_type, float pos_x, float pos_y);

	static void update(uint64_t time);
	static void queue_remove(int entity_num);
	static void remove_marked();
	static void remove_all();
	static void draw(sf::RenderWindow& window);
private:
	EntityManager();
	static std::vector<EntityPtr_t> all_entities_;
	static std::set<int> remove_entities_;
};