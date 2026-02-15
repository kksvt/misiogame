#pragma once
#include <SFML/Window.hpp>
#include <Box2D/Box2D.h>
#include "debugdraw.h"
#include <unordered_map>

class Entity;

struct PhysicsManager {
    static float pixels_per_meter;
    static float meters_per_pixel;
    static b2WorldId world_id;
    static void init(float gravity, float pixels_per_meter, float meters_per_pixel, int sub_step_count);
    static void init_debug_draw(sf::RenderWindow& window);
    static void run(float dt, uint64_t total_time);
    static void draw();
    static void destroy();
    static void create_physical_entities_for_tiles(const sf::Vector2u &render_size);
    static void queue_movement(Entity* ent, const sf::Vector2f &desired_movement);
    static void remove_movement(Entity* ent);
    static void clear_queue();
private:
    PhysicsManager();
    static int sub_step_count;
    static SFMLDebugDrawContext ctx;
    static b2DebugDraw debug_draw;
    static std::unordered_map<Entity*, sf::Vector2f> move;
};