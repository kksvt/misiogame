#pragma once

#include <string>
#include "PhysicsManager.h"
#include "TextureManager.h"
#include "EntityManager.h"

enum SerializedEntityType_t {
	SET_PLAYER = 0,
	SET_ALIEN_PURPLE,
	SET_ALIEN_BLUE,
	SET_SPIKES,
	SET_TILE_FIRST,
	SET_TILE_LAST = SET_TILE_FIRST + GT_CORNER_BOTTOM_RIGHT,
	SET_DOOR,
	SET_KEY,
	SET_MEDPAK,
	SET_INFO,
	SET_FOOD,
	SET_MAX,
};

class Level {
private:
	bool is_editor_;
	bool debug_shapes_;
	std::string file_path_;
	std::string background_path_;
	sf::RenderWindow* render_;
	int64_t current_blueprint_;
	Entity* current_entity_;

	void regular_loop();
	void editor_loop();

public:
	Level(bool is_editor, bool debug_shapes, std::string file_path, std::string background_path, sf::RenderWindow* window);
	void loop();
};