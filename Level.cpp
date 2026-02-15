#include "Level.h"

#include <iostream>
#include <unordered_map>

static Entity* EntityFromSerializableEnum(uint8_t type, bool physics, float pos_x, float pos_y) {
    switch (type) {
    case SET_PLAYER:
        return EntityManager::create_concrete_player(physics, pos_x, pos_y);
    case SET_ALIEN_PURPLE:
        return EntityManager::create_baddie_one(physics, pos_x, pos_y);
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
        if (type >= SET_TILE_FIRST &&
            type <= SET_TILE_LAST) {
            return EntityManager::create_grass_tile(type - SET_TILE_FIRST, pos_x, pos_y);
        }
    }
    return nullptr;
}

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

    load(true);

    PhysicsManager::create_physical_entities_for_tiles();

    Entity* player = EntityManager::get_entity(0);
    if (!player || player->get_serializable_type() != SET_PLAYER) {
        std::runtime_error("Level has no player entity OR the player entity is not the first entity.");
    }

    EntityManager::create_background("background.png", 640, 480);

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
            player->get_physics()->move(desired_movement, 8.f, 8.f);
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
    bool is_paused = false;

    std::cout << "Level editor mode is active.\n" << "Use the mouse wheel to scroll between placeable entities.\n" <<
        "Press Left click to place an entity.\n" << "Press Right click to delete an entity.\n" << 
        "Press mouse wheel to copy the entity you're currently hovering over.\n" <<
        "Press S to save the level file.\n";

    //make sure player is id 0
    Entity* player = nullptr;

    if (!load(false)) {
        player = EntityManager::create_concrete_player(false, 0.f, 0.f);
    }
    else {
        player = EntityManager::get_entity(0);
        ++current_blueprint_;
    }

    while (render_->isOpen())
    {
        auto last_blueprint = current_blueprint_;
        while (const std::optional event = render_->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                render_->close();
            else if (event->is<sf::Event::FocusLost>()) {
                is_paused = true;
            }
            else if (event->is<sf::Event::FocusGained>()) {
                is_paused = false;
            }
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
            else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::S) {
                    save();
                }
            }
        }

        if (is_paused) {
            continue;
        }

        auto mouse = sf::Mouse::getPosition(*render_);

        Entity* hovered_entity = EntityManager::get_entity(mouse.x, mouse.y, current_entity_ ? current_entity_->get_id() : -1);

        if (!hovered_entity && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (current_blueprint_ == SET_PLAYER) {
                ++current_blueprint_;
            }
            current_entity_ = nullptr;
        }

        if (hovered_entity) {
            if (hovered_entity->get_id() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                EntityManager::queue_remove(hovered_entity->get_id());
                auto pos = hovered_entity->get_position();
                std::cout << "Deleting entity @ " << pos.x << ", " << pos.y << '\n';
                hovered_entity = nullptr;
            }
            else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
                current_blueprint_ = hovered_entity->get_serializable_type();
            }
        }

        if (current_entity_ && current_entity_->get_id() && last_blueprint != current_blueprint_) {
            EntityManager::queue_remove(current_entity_->get_id());
            current_entity_ = nullptr;
        }

        if (!current_entity_) {
            Entity* ent = nullptr;
            if (current_blueprint_ == SET_PLAYER) {
                ent = player;
            }
            else {
                ent = EntityFromSerializableEnum(current_blueprint_, false, 0., 0.);
            }

            if (ent) {
                current_entity_ = ent;
            }
        }

        if (current_entity_) {
            auto snap = mouse;
            //hacky, but the player sprite doesnt fit a 32x32 grid and i dont have the time to redo the sprite
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

bool Level::save()
{
    //file structure:
    //num entities (uint32)
    //for each entity:
    //  entitytype (uint8), pos_x(float), pos_y(float)
    FILE* f = NULL;
    if (fopen_s(&f, file_path_.c_str(), "wb")) {
        std::cerr << "Couldn't open file " << file_path_ << " for writing\n";
        return false;
    }

    uint32_t num_entities = EntityManager::get_num_active_entities();

    if (current_entity_ && current_entity_->get_serializable_type() != SET_PLAYER) {
        --num_entities;
    }

    fwrite(&num_entities, sizeof(uint32_t), 1, f);

    for (uint32_t i = 0, total = EntityManager::get_num_allocated_entities(); i < total; ++i) {
        Entity* ent = EntityManager::get_entity(i);
        if (!ent || (ent == current_entity_ && ent->get_serializable_type() != SET_PLAYER)) {
            continue;
        }
        auto type = ent->get_serializable_type();
        auto pos = ent->get_position();

        std::cout << "Saving entity number " << i << ", name: " << ent->get_name() <<
            ", pos: (" << pos.x << ", " << pos.y << "), serializable type: " << static_cast<int>(type) << "\n";

        fwrite(&type, sizeof(type), 1, f);
        fwrite(&pos.x, sizeof(pos.x), 1, f);
        fwrite(&pos.y, sizeof(pos.y), 1, f);
    }

    std::cout << "Level saved.\n";
    fclose(f);
    return true;
}

bool Level::load(bool physics)
{
    uint32_t num_entities;
    FILE* f = NULL;
    if (fopen_s(&f, file_path_.c_str(), "rb")) {
        std::cerr << "Couldn't open file " << file_path_ << " for reading\n";
        return false;
    }

    fread(&num_entities, 1, sizeof(num_entities), f);
    std::cout << "Attempting to read " << num_entities << " from the level file...\n";

    uint8_t type;
    float pos_x;
    float pos_y;

    for (uint32_t i = 0; i < num_entities; ++i) {
        if (fread(&type, sizeof(type), 1, f) != 1 ||
            fread(&pos_x, sizeof(pos_x), 1, f) != 1 ||
            fread(&pos_y, sizeof(pos_y), 1, f) != 1) {
            throw std::runtime_error("Level file is corrupted.");
        }
        std::cout << /*"  entity: " << static_cast<int>(type) <<*/ " @ (" << pos_x << ", " << pos_y << ")\n";
        if (EntityManager::get_entity(pos_x, pos_y, -1)) {
            std::cout << "   > trying to place it on top of another entity?! Skipping...\n";
            continue;
        }
        EntityFromSerializableEnum(type, physics, pos_x, pos_y);
    }

    std::cout << "Level parsed.\n";
    fclose(f);
    return true;
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
