#pragma once

#include <string>
#include "PhysicsManager.h"
#include "TextureManager.h"
#include "EntityManager.h"

class Level {
private:
	bool is_editor_;
	bool debug_shapes_;
	std::string file_path_;
	std::string background_path_;
	sf::RenderWindow* render_;
	int16_t current_blueprint_;
	Entity* current_entity_;

	void regular_loop();
	void editor_loop();

	bool save();
	bool load(bool physics);
public:
	Level(bool is_editor, bool debug_shapes, std::string file_path, std::string background_path, sf::RenderWindow* window);
	void loop();
};