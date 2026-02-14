#include "Level.h"

#include <iostream>

void Level::regular_loop()
{
    const float fixed_dt = 1.f / 60.f;
    const float maxframetime = 0.25f;
    const int max_steps_per_frame = 8;
    const float pixels_per_meter = 40.f;
    const float meters_per_pixel = 1.0f / pixels_per_meter;

    sf::Clock game_clock;

    int steps;

    float acc = 0.f, dt, frametime;

    auto last_time = game_clock.getElapsedTime().asMilliseconds();

    PhysicsManager::init(9.8f, pixels_per_meter, meters_per_pixel, 4);
    if (debug_shapes_) {
        PhysicsManager::init_debug_draw(*render_);
    }

    //PhysicsComponentPtr_t obj2(new PhysicsComponent_t(
    //    PhysicsManager::world_id, nullptr, false, 768.f, 64.f, 0.f, 256.f, 1.f, .0f
    //));

    //Entity* player = EntityManager::create_player("misio.png", 20, 35, 20, 35, 0, 0, { 6, 6 }, { 100, 100 });

    EntityManager::create_background("background.png", 640, 480);

    //Entity* enemy = EntityManager::;

    auto total_time = static_cast<uint32_t>(0);
    auto next_decelerate = static_cast<uint32_t>(0);

    while (render_->isOpen())
    {
        while (const std::optional event = render_->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                render_->close();
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
            //player->get_physics()->move(desired_movement, 8.f, 8.f);
            PhysicsManager::run(fixed_dt);
        }

        if (steps >= max_steps_per_frame) {
            acc = 0.f;
        }

        render_->clear();

        EntityManager::update(total_time);
        EntityManager::draw(*render_);
        if (debug_shapes_) {
            PhysicsManager::draw();
        }
        render_->display();
        EntityManager::remove_marked();
        last_time = total_time;

    }

    EntityManager::remove_all();
    PhysicsManager::destroy();
}

void Level::editor_loop()
{
    //make sure player is id 0
    Entity* player = EntityManager::create_concrete_player(false, 0.f, 0.f);
    bool sync_player = true;

    std::cout << "Level editor mode is active.\n" << "Use the mouse wheel to scroll between placable entities\n" <<
        "Press Left click to place an entity.\n" << "Press Right click to delete an entity.\n" << 
        "Press S to save the level file.\n";

    while (render_->isOpen())
    {
        auto last_blueprint = current_blueprint_;
        while (const std::optional event = render_->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                render_->close();
            else if (const auto* mouse_wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouse_wheel->delta > 0.f) {
                    if (++current_blueprint_ >= SET_MAX) {
                        current_blueprint_ = SET_PLAYER;
                    }
                }
                else if (mouse_wheel->delta < 0.f) {
                    if (--current_blueprint_ < 0) {
                        current_blueprint_ = SET_MAX - 1;
                    }
                }
            }
        }

        auto mouse = sf::Mouse::getPosition(*render_);

        Entity* hovered_entity = EntityManager::get_entity(mouse.x, mouse.y, current_entity_ ? current_entity_->get_id() : -1);

        if (!hovered_entity && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (current_blueprint_ == SET_PLAYER) {
                ++current_blueprint_;
            }
            current_entity_ = nullptr;
        }

        if (hovered_entity && hovered_entity->get_id() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            EntityManager::queue_remove(hovered_entity->get_id());
            hovered_entity = nullptr;
        }

        if (current_entity_ && current_entity_->get_id() && last_blueprint != current_blueprint_) {
            EntityManager::queue_remove(current_entity_->get_id());
            current_entity_ = nullptr;
        }

        if (!current_entity_) {
            Entity* ent = nullptr;
            switch (current_blueprint_) {
            case SET_PLAYER:
                ent = player;
                break;
            case SET_ALIEN_PURPLE:
                break;
            case SET_ALIEN_BLUE:
                break;
            case SET_SPIKES:
                break;
            case SET_DOOR:
                break;
            case SET_KEY:
                break;
            case SET_MEDPAK:
                break;
            case SET_INFO:
                break;
            case SET_FOOD:
                break;
            default:
                if (current_blueprint_ >= SET_TILE_FIRST &&
                    current_blueprint_ <= SET_TILE_LAST) {
                    ent = EntityManager::create_grass_tile(current_blueprint_ - SET_TILE_FIRST, 0.f, 0.f);
                }
                break;
            }

            if (ent) {
                current_entity_ = ent;
            }
        }

        if (current_entity_) {
            auto snap = mouse;
            //hacky
            if (current_blueprint_ != SET_PLAYER) {
                auto coeff_x = snap.x / 32;
                auto coeff_y = snap.y / 32;
                snap.x = 32 * coeff_x;
                snap.y = 32 * coeff_y;
            }
            current_entity_->get_sprite()->set_position({ static_cast<float>(snap.x), static_cast<float>(snap.y) });

        }

        render_->clear();

        EntityManager::draw(*render_);

        render_->display();

        EntityManager::remove_marked();
    }
}

Level::Level(bool is_editor, bool debug_shapes, 
    std::string file_path, std::string background_path, sf::RenderWindow* window)
	: is_editor_(is_editor), debug_shapes_(debug_shapes), file_path_(file_path),
    background_path_(background_path), render_(window), current_blueprint_(SET_PLAYER), current_entity_(nullptr)
{

}

void Level::loop()
{
    if (is_editor_) {
        editor_loop();
    }
    else {
        regular_loop();
    }
}
