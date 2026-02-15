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
    Level level1(false, true, "level_debug.dat", "background.png", &window);
    level1.loop();
    return 0;
}