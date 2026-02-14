#pragma once
#include <string>
#include <unordered_map>
#include <memory>
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

	Entity(uint32_t id, std::string name) : id_(id), name_(name), entity_type_(ENT_TYPE_NONE) {}

public:
	void update(uint64_t time);
	PhysicsComponent_t* get_physics() { return physics_.get(); }
	SpriteComponent_t* get_sprite() { return sprite_.get(); }

	~Entity();

};

using EntityPtr_t = std::unique_ptr<Entity>;