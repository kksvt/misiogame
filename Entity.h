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
	uint32_t entity_type_;
	PhysicsComponentPtr_t physics_;
	SpriteComponentPtr_t sprite_;
	std::function<void(Entity*, uint64_t)> process_callback_;

	Entity(uint32_t id, std::string name, uint8_t type) : id_(id), name_(name), entity_type_(type) {}

public:
	void update(uint64_t time);
	PhysicsComponent_t* get_physics() { return physics_.get(); }
	SpriteComponent_t* get_sprite() { return sprite_.get(); }
	void set_process_callback(std::function<void(Entity*, uint64_t)> func);
	uint32_t get_type() const { return entity_type_; }
	uint32_t get_id() const { return id_; }

	~Entity();

};

using EntityPtr_t = std::unique_ptr<Entity>;