#pragma once
#include <memory>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include "PhysicsManager.h"

class Entity;

class PhysicsComponent_t {
private:
	b2BodyDef body_def_;
	b2BodyId body_id_;
	b2Polygon body_polygon_;
	b2ShapeDef body_shape_def_;
	float half_width_;
	float half_height_;
	uint64_t last_moved_;

public:
	PhysicsComponent_t(
		const b2WorldId& world_id, Entity* ent,
		bool is_dynamic, float width, float height, 
		float pos_x, float pos_y, float density = 1.0f, float friction = 0.3f);

	~PhysicsComponent_t();

	sf::Vector2f get_position();

	sf::Vector2f get_hitbox();

	void apply_force(const sf::Vector2f& force);

	void move(const sf::Vector2f& dir, float run_speed, float jump_speed);

	b2Vec2 get_velocity() { return b2Body_GetLinearVelocity(body_id_); };
};

using PhysicsComponentPtr_t = std::unique_ptr<PhysicsComponent_t>;