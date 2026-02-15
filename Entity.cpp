#include "Entity.h"

void Entity::update(uint64_t time) {
	if (sprite_) {
		sprite_->update(time);
		if (physics_) {
			if (entity_type_ == EntityType_t::ET_CHARACTER) {
				auto v = physics_->get_velocity();
				if (std::fabs(v.x) < 0.1f) {
					sprite_->set_row(0);
				}
				else if (v.x > 0.1f) { //an epsilon would probably be better here
					sprite_->set_flipped(false, false);
					if (sprite_->get_rows() > 1) {
						sprite_->set_row(1);
					}
				}
				else if (v.x < -0.1f) {
					sprite_->set_flipped(true, false);
					if (sprite_->get_rows() > 1) {
						sprite_->set_row(1);
					}
				}
			}
			sprite_->set_position(physics_->get_position());
		}
	}
	if (process_callback_) {
		process_callback_(this, time);
	}
}

void Entity::set_process_callback(std::function<void(Entity*, uint64_t)> func)
{
	process_callback_ = func;
}

void Entity::set_touch(std::function<void(Entity*, Entity*, uint64_t total_time, b2ManifoldPoint[2], int, b2Vec2*)> func)
{
	touch_callback_ = func;
}

void Entity::touch(Entity* other, uint64_t total_time, b2ManifoldPoint points[2], const int num_points, b2Vec2* normal)
{
	if (touch_callback_) {
		touch_callback_(this, other, total_time, points, num_points, normal);
	}
}

void Entity::set_stop_touch(std::function<void(Entity*, Entity*)> func)
{
	stop_touch_callback_ = func;
}

void Entity::stop_touch(Entity* other)
{
	if (stop_touch_callback_) {
		stop_touch_callback_(this, other);
	}
}

void Entity::set_sensor_touch(std::function<void(Entity*, Entity*)> func)
{
	sensor_touch_callback_ = func;
}

void Entity::sensor_touch(Entity* other)
{
	if (sensor_touch_callback_) {
		sensor_touch_callback_(this, other);
	}
}

void Entity::set_sensor_stop_touch(std::function<void(Entity*, Entity*)> func)
{
	sensor_stop_touch_callback_ = func;
}

void Entity::sensor_stop_touch(Entity* other)
{
	if (sensor_stop_touch_callback_) {
		sensor_stop_touch_callback_(this, other);
	}
}

sf::Vector2f Entity::get_position() {
	if (physics_) {
		return physics_->get_position();
	}
	if (sprite_) {
		return sprite_->get_position();
	}
	return sf::Vector2f(0.f, 0.f);
}

void Entity::add_string_data(const std::string& s)
{
	auto str = new StringComponent_t();
	str->string = s;
	str_.reset(str);
}

#include <iostream>

Entity::~Entity()
{
	if (sprite_) {
		sprite_.reset();
	}
	if (physics_) {
		physics_.reset();
	}
	if (ai_) {
		ai_.reset();
	}
	if (str_) {
		str_.reset();
	}
}
