#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "EntityType.h"
#include "PhysicsComponent.h"
#include "SpriteComponent.h"

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
	std::function<void(Entity*, uint64_t)> process_callback_;

	Entity(uint32_t id, std::string name, uint8_t type, uint8_t set_type) 
		: id_(id), name_(name), entity_type_(type), serializable_entity_type_(set_type) {}

public:
	void update(uint64_t time);
	PhysicsComponent_t* get_physics() { return physics_.get(); }
	SpriteComponent_t* get_sprite() { return sprite_.get(); }
	void set_process_callback(std::function<void(Entity*, uint64_t)> func);
	uint8_t get_type() const { return entity_type_; }
	uint8_t get_serializable_type() const { return serializable_entity_type_; }
	uint32_t get_id() const { return id_; }
	std::string get_name() const { return name_; }
	sf::Vector2f get_position();

	~Entity();

};

using EntityPtr_t = std::unique_ptr<Entity>;