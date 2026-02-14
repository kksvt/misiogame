#include "SpriteComponent.h"
#include <iostream>
#include <stdexcept>

SpriteComponent_t::SpriteComponent_t(const std::string& texture_name, 
	const sf::IntRect& frame_rectangle, 
	const std::initializer_list<uint8_t>& num_frames, 
	const std::initializer_list<uint8_t>& animation_speed) : 
	current_row_(0), current_frame_(0), flipped_(false), last_animation_update_(0), frame_rectangle_(frame_rectangle)
{
	if (!animation_speed.size()) {
		std::cerr << "No animation speed provided for sprite " << texture_name << ", defaulting to 100\n";
		animation_speed_.push_back(100);
	}
	else {
		animation_speed_.insert(animation_speed_.end(), animation_speed);
	}

	if (!num_frames.size()) {
		std::cerr << "No number of frames provided for sprite " << texture_name << ", defaulting to 1\n";
		num_frames_.push_back(1);
	}
	else {
		num_frames_.insert(num_frames_.end(), num_frames);
	}

	auto tex = TextureManager::get_texture(texture_name);
	if (!tex) {
		if (!TextureManager::load_texture(texture_name)) {
			throw std::runtime_error("Invalid texture " + texture_name);
		}
		tex = TextureManager::get_texture(texture_name);
	}

	if (tex) {
		sprite_.emplace(*tex, frame_rectangle_);
	}
}

void SpriteComponent_t::set_position(const sf::Vector2f& pos)
{
	if (flipped_) {
		sprite_->setPosition({pos.x + frame_rectangle_.size.x, pos.y });
	}
	else {
		sprite_->setPosition(pos);
	}
}

void SpriteComponent_t::update(uint64_t time)
{
	auto speed = animation_speed_[current_row_ % animation_speed_.size()];
	//don't update the animation if speed is 0 - this will come in handy for sheets with ground tiles
	if (!speed) {
		return;
	}
	auto dt = time - last_animation_update_;
	if (dt > speed) {
		current_frame_ = (current_frame_ + 1) % num_frames_[current_row_ % num_frames_.size()];
		last_animation_update_ = time;
		frame_rectangle_.position.x = frame_rectangle_.size.x * current_frame_;
		sprite_->setTextureRect(frame_rectangle_);
	}
}

void SpriteComponent_t::set_row(uint8_t row)
{
	current_row_ = row % num_frames_[current_row_ % num_frames_.size()];
	frame_rectangle_.position.y = frame_rectangle_.size.y * current_row_;
	sprite_->setTextureRect(frame_rectangle_);
}

void SpriteComponent_t::set_square(uint8_t square)
{
	current_frame_ = (current_frame_ + 1) % num_frames_[current_row_ % num_frames_.size()];
	frame_rectangle_.position.x = frame_rectangle_.size.x * current_frame_;
	sprite_->setTextureRect(frame_rectangle_);
}

void SpriteComponent_t::set_flipped(bool flipped)
{
	auto scale = sprite_->getScale();
	scale.x = std::abs(scale.x) * (flipped ? -1.f : 1.f);
	sprite_->setScale(scale);
	flipped_ = flipped;
}
