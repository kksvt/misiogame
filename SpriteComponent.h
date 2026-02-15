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
	bool flipped_x_;
	bool flipped_y_;
	uint8_t drawing_priority_;

public:
	SpriteComponent_t(const std::string& texture_name,
		const sf::IntRect& frame_rectangle,
		const std::initializer_list<uint8_t>& num_frames,
		const std::initializer_list<uint8_t>& animation_speed,
		uint8_t drawing_priority);

	bool valid_sprite() { return sprite_.has_value(); }

	sf::Sprite& get_sprite() { return *sprite_; }

	void set_position(const sf::Vector2f& pos);

	sf::Vector2f get_position();

	void update(uint64_t time);

	void set_row(uint8_t row);

	void set_square(uint8_t square); //note: the results will be overwriten by SpriteComponent_t::update if speed is non-zero

	void set_flipped(bool flipped_x, bool flipped_y);

	bool get_flipped_x() const { return flipped_x_; }

	bool get_flipped_y() const { return flipped_y_; }

	uint8_t get_priority() const { return drawing_priority_; }

	sf::Vector2i get_size() { return frame_rectangle_.size; }

	size_t get_rows() { return num_frames_.size(); }

	void set_animation_speed(const std::initializer_list<uint8_t>& animation_speed);

	uint8_t get_animation_speed() { return animation_speed_[current_row_ % animation_speed_.size()]; };

	uint8_t get_frame() { return current_frame_; }
};

using SpriteComponentPtr_t = std::unique_ptr<SpriteComponent_t>;