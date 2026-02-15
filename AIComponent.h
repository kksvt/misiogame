#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

class Entity;

struct AIComponent_t {
	sf::Vector2f desired_movement;
	uint64_t last_contact;
};

using AIComponentPtr_t = std::unique_ptr<AIComponent_t>;