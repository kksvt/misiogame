#include "Entity.h"

#include <iostream>
#include <vector>

#include <box2d\box2d.h>
#include "PhysicsManager.h"
#include "TextureManager.h"
#include "EntityManager.h"

#include "SpriteComponent.h"

const float pixels_per_meter = 30.f;
const float meters_per_pixel = 1.0f / pixels_per_meter;

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "SFML works!");
    window.setFramerateLimit(60);
    sf::Clock game_clock;

    const float fixed_dt = 1.f / 60.f;
    const float maxframetime = 0.25f;
    const int max_steps_per_frame = 8;

    int steps;

    float acc = 0.f, dt, frametime;
    
    auto last_time = game_clock.getElapsedTime().asMilliseconds();

    b2Version version = b2GetVersion();
    printf("Box2D version %d.%d.%d\n", version.major, version.minor, version.revision);

    PhysicsManager::init(9.8f, pixels_per_meter, meters_per_pixel, 4);
    //PhysicsManager::init_debug_draw(window);

    //PhysicsComponentPtr_t obj2(new PhysicsComponent_t(
    //    PhysicsManager::world_id, false, 768.f, 64.f, 0.f, 256.f, 1.f, .0f
    //));

    Entity* player = EntityManager::create_player("misio.png", 20, 35, 20, 35, 0, 0, { 6, 6 }, { 100, 100 });
    bool player_moved = false;

    auto total_time = static_cast<uint32_t>(0);
    auto next_decelerate = static_cast<uint32_t>(0);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        
        total_time = game_clock.getElapsedTime().asMilliseconds();

        dt = (total_time - last_time) / 1000.f;
        frametime = std::min(maxframetime, dt);
        acc += frametime;

        auto desired_movement = sf::Vector2f();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
            desired_movement.x += 1.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
            desired_movement.x -= 1.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
            desired_movement.y -= 1.f;
        }

        for (steps = 0; acc >= fixed_dt && steps < max_steps_per_frame; ++steps, acc -= fixed_dt) {
            player->get_physics()->move(desired_movement, 8.f, 8.f);
            PhysicsManager::run(fixed_dt);
        }

        if (steps >= max_steps_per_frame) {
            acc = 0.f;
        }

        window.clear();

        EntityManager::update(total_time);
        PhysicsManager::draw();
        EntityManager::draw(window);
        window.display();
        EntityManager::remove_marked();
        last_time = total_time;

    }

    EntityManager::remove_all();
    PhysicsManager::destroy();
}