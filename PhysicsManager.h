#pragma once
#include <SFML/Window.hpp>
#include <Box2D/Box2D.h>
#include "debugdraw.h"

struct PhysicsManager {
    static float pixels_per_meter;
    static float meters_per_pixel;
    static b2WorldId world_id;
    static void init(float gravity, float pixels_per_meter, float meters_per_pixel, int sub_step_count);
    static void init_debug_draw(sf::RenderWindow& window);
    static void run(float dt);
    static void draw();
    static void destroy();
private:
    PhysicsManager();
    static int sub_step_count;
    static SFMLDebugDrawContext ctx;
    static b2DebugDraw debug_draw;
};