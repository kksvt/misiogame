#include "Entity.h"

void Entity::update(uint64_t time) {
	if (sprite_) {
		sprite_->update(time);
		if (physics_) {
			auto v = physics_->get_velocity();
			if (std::fabs(v.x) < 0.1f) {
				sprite_->set_row(0);
			}
			else if (v.x > 0.1f) { //an epsilon would probably be better here
				sprite_->set_flipped(false);
				sprite_->set_row(1);
			}
			else if (v.x < -0.1f) {
				sprite_->set_flipped(true);
				sprite_->set_row(1);
			}
			sprite_->set_position(physics_->get_position());
		}
	}
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
