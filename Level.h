#pragma once

#include <string>
#include "PhysicsManager.h"
#include "TextureManager.h"
#include "EntityManager.h"

enum LevelState_t {
	LS_EDITOR = 0,
	LS_NORMAL,
	LS_SPLASH_SCREEN,
};

class Level {
private:
	uint8_t level_state_;
	bool debug_shapes_;
	bool paused_; //if an actual level gets paused, it should not get unpaused. it exists only to provide the fade to black functionality
	bool quit_;
	std::string file_path_;
	std::string background_path_;
	sf::RenderWindow* render_;
	int16_t current_blueprint_;
	Entity* current_entity_;
	uint8_t num_food_cans_;
	Level* next_level_;
	Level* fallback_level_;
	Level* queued_level_;
	std::string tooltip_text_;

	uint8_t collected_keys_;
	uint8_t collected_score_;
	uint8_t collected_lives_;

	void regular_loop();
	void editor_loop();
	void splash_loop();

	bool save();
	bool load(bool physics);

public:
	Level(uint8_t state, bool debug_shapes, std::string file_path, std::string background_path, sf::RenderWindow* window);
	void loop();
	void set_next_level(Level* level);
	void set_fallback_level(Level* level);
	void change_level(Level* level);
	Level* get_next_level() { return next_level_; }
	Level* get_fallback_level() { return fallback_level_;  }
	void set_tooltip_text(const std::string& s);

	void collect_key();
	void collect_food();
	void collect_life();

	uint8_t get_collected_keys() const { return collected_keys_;  }
	uint8_t get_collected_food() const { return collected_score_; }
	uint8_t get_collected_lives() const { return collected_lives_; }
};