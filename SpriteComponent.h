#pragma once
#include "TextureManager.h"
#include <memory>
#include <vector>

class SpriteComponent_t {
private:
	sf::IntRect frame_rectangle_;
	std::optional<sf::Sprite> sprite_;
	uint64_t last_animation_update_;
	std::vector<uint8_t> num_frames_;
	std::vector<uint8_t> animation_speed_;
	uint8_t current_row_;
	uint8_t current_frame_;
	bool flipped_;

public:
	SpriteComponent_t(const std::string& texture_name,
		const sf::IntRect& frame_rectangle,
		const std::initializer_list<uint8_t>& num_frames,
		const std::initializer_list<uint8_t>& animation_speed);

	bool valid_sprite() { return sprite_.has_value(); }

	sf::Sprite& get_sprite() { return *sprite_; }

	void set_position(const sf::Vector2f& pos);

	void update(uint64_t time);

	void set_row(uint8_t row);

	void set_square(uint8_t square); //note: the results will be overwriten by SpriteComponent_t::update if speed is non-zero

	void set_flipped(bool flipped);

	bool get_flipped() { return flipped_; }
};

using SpriteComponentPtr_t = std::unique_ptr<SpriteComponent_t>;