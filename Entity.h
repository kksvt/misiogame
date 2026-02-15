#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "EntityType.h"
#include "PhysicsComponent.h"
#include "SpriteComponent.h"
#include "AIComponent.h"
#include "StringComponent.h"

class EntityManager;

class Entity {
	friend class EntityManager;
private:
	uint32_t id_;
	std::string name_;
	uint8_t entity_type_;
	uint8_t serializable_entity_type_;
	PhysicsComponentPtr_t physics_;
	SpriteComponentPtr_t sprite_;
	AIComponentPtr_t ai_;
	StringComponentPtr_t str_;

	std::function<void(Entity*, uint64_t)> process_callback_;
	std::function<void(Entity*, Entity*, uint64_t total_time, b2ManifoldPoint[2], int, b2Vec2*)> touch_callback_;
	std::function<void(Entity*, Entity*)> stop_touch_callback_;
	std::function<void(Entity*, Entity*)> sensor_touch_callback_;
	std::function<void(Entity*, Entity*)> sensor_stop_touch_callback_;

	Entity(uint32_t id, std::string name, uint8_t type, uint8_t set_type) 
		: id_(id), name_(name), entity_type_(type), serializable_entity_type_(set_type) {}

public:
	void update(uint64_t time);
	PhysicsComponent_t* get_physics() { return physics_ ? physics_.get() : nullptr; }
	SpriteComponent_t* get_sprite() { return sprite_ ? sprite_.get() : nullptr; }
	AIComponent_t* get_ai() { return ai_ ? ai_.get() : nullptr; }
	StringComponent_t* get_str() { return str_ ? str_.get() : nullptr; }
	void set_process_callback(std::function<void(Entity*, uint64_t)> func);

	void set_touch(std::function<void(Entity*, Entity*, uint64_t total_time, b2ManifoldPoint[2], int, b2Vec2*)> func);
	void touch(Entity*, uint64_t total_time, b2ManifoldPoint points[2], int num_points, b2Vec2* normal);

	void set_stop_touch(std::function<void(Entity*, Entity*)> func);
	void stop_touch(Entity*);

	void set_sensor_touch(std::function<void(Entity*, Entity*)> func);
	void sensor_touch(Entity*);

	void set_sensor_stop_touch(std::function<void(Entity*, Entity*)> func);
	void sensor_stop_touch(Entity*);

	uint8_t get_type() const { return entity_type_; }
	uint8_t get_serializable_type() const { return serializable_entity_type_; }
	uint32_t get_id() const { return id_; }
	std::string get_name() const { return name_; }
	sf::Vector2f get_position();

	void add_string_data(const std::string& s);

	~Entity();

};

using EntityPtr_t = std::unique_ptr<Entity>;