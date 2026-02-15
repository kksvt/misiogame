#include "Entity.h"

#include <iostream>
#include <vector>

#include <box2d\box2d.h>
#include "PhysicsManager.h"
#include "TextureManager.h"
#include "EntityManager.h"
#include "Level.h"

#include "SpriteComponent.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "Misio Demo");
    window.setFramerateLimit(60);

    uint8_t state = LevelState_t::LS_NORMAL;

    if (state == LevelState_t::LS_NORMAL || 
        state == LevelState_t::LS_SPLASH_SCREEN) {
        Level level1(state, false, "levels/level1.dat", "sprites/background.png", &window);
        Level level2(state, false, "levels/level2.dat", "sprites/background.png", &window);
        Level level3(state, false, "levels/level3.dat", "sprites/background.png", &window);
        Level level4(state, false, "levels/level4.dat", "sprites/background.png", &window);
        Level level5(state, false, "levels/thank_you.dat", "sprites/background.png", &window);

        Level game_over(LevelState_t::LS_SPLASH_SCREEN, false, "", "sprites/gameover.png", &window);

        level1.set_next_level(&level2);
        level2.set_next_level(&level3);
        level3.set_next_level(&level4);
        level4.set_next_level(&level5);
        level5.set_next_level(&level1);
        game_over.set_next_level(&level1);

        EntityManager::current_level = &level1;
        EntityManager::num_lives = 3;
        EntityManager::score = 0;
        EntityManager::num_keys = 0;
        do {
            EntityManager::current_level->set_fallback_level(&game_over);
            EntityManager::current_level->loop();
        } while (EntityManager::current_level);
    }
    else if (state == LevelState_t::LS_EDITOR) {
        Level editing(state, false, "levels/level4.dat", "", &window);
        editing.loop();
    }
    return 0;
}