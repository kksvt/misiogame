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
	b2ShapeId body_shape_id_;
	float half_width_;
	float half_height_;
	int16_t ground_contacts_;

public:
	PhysicsComponent_t(
		const b2WorldId& world_id, Entity* ent,
		bool is_dynamic, float width, float height, 
		float pos_x, float pos_y, bool enable_events, float density = 1.0f, float friction = 0.3f);

	~PhysicsComponent_t();

	sf::Vector2f get_position();

	sf::Vector2f get_hitbox();

	void apply_force(const sf::Vector2f& force);

	void move(const sf::Vector2f& dir/*, float run_speed, float jump_speed*/);
	
	void set_grounded(int16_t contacts) { ground_contacts_ = contacts; }

	void increase_contacts() { ++ground_contacts_; }

	void decrease_contacts() { --ground_contacts_; }

	//applicable only to dynamic bodies
	int16_t get_grounded() const { return ground_contacts_; }

	b2Vec2 get_velocity() { return b2Body_GetLinearVelocity(body_id_); }

	b2BodyId get_body_id() { return body_id_; }

	b2ShapeId get_shape_id() { return body_shape_id_; }

	float get_sim_half_width() const { return half_width_ * PhysicsManager::meters_per_pixel; }

	float get_sim_half_height() const { return half_height_ * PhysicsManager::meters_per_pixel; }
};

using PhysicsComponentPtr_t = std::unique_ptr<PhysicsComponent_t>;