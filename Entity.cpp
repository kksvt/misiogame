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
					sprite_->set_row(1);
				}
				else if (v.x < -0.1f) {
					sprite_->set_flipped(true, false);
					sprite_->set_row(1);
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

sf::Vector2f Entity::get_position() {
	if (physics_) {
		return physics_->get_position();
	}
	if (sprite_) {
		return sprite_->get_position();
	}
	return sf::Vector2f(0.f, 0.f);
}

Entity::~Entity()
{
	if (sprite_) {
		sprite_.reset();
	}
	if (physics_) {
		physics_.reset();
	}
}
