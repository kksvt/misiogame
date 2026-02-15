#include "Level.h"

#include <iostream>
#include <unordered_map>

static bool FormatUI(sf::Text &text, uint8_t num_lives, uint8_t num_keys, uint8_t score) {
    static char s[256] = { 0 };
    static uint8_t saved_lives = 0;
    static uint8_t saved_keys = 0;
    static uint8_t saved_score = 0;

    if (!text.getString().isEmpty() &&
        num_lives == saved_lives &&
        num_keys == saved_keys &&
        score == saved_score) {
        return false;
    }
    std::snprintf(s, sizeof(s), "Lives: %3u    Keys: %3u    Score: %3u", num_lives, num_keys, score);
    saved_lives = num_lives;
    saved_keys = num_keys;
    saved_score = score;
    text.setString(s);
    return true;
}

static Entity* EntityFromSerializableEnum(uint8_t type, bool physics, float pos_x, float pos_y) {
    switch (type) {
    case SET_PLAYER:
        return EntityManager::create_concrete_player(physics, pos_x, pos_y);
    case SET_ALIEN_PURPLE:
        return EntityManager::create_baddie_one(physics, pos_x, pos_y);
    case SET_ALIEN_BLUE:
        return EntityManager::create_baddie_one(physics, pos_x, pos_y); //placeholder
    case SET_SPIKES:
        return EntityManager::create_spikes(physics, pos_x, pos_y);
    case SET_DOOR_LOCKED:
        return EntityManager::create_door_locked(physics, pos_x, pos_y);
    case SET_DOOR_EXIT:
        return EntityManager::create_door_exit(physics, pos_x, pos_y);
    case SET_KEY:
        return EntityManager::create_key(physics, pos_x, pos_y);
    case SET_MEDPAK:
        return EntityManager::create_medpak(physics, pos_x, pos_y);
    case SET_INFO:
        return EntityManager::create_info_sign(physics, pos_x, pos_y);
    case SET_FOOD:
        return EntityManager::create_food(physics, pos_x, pos_y);
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
    const float pixels_per_meter = 32.f;
    const float meters_per_pixel = 1.0f / pixels_per_meter;

    float player_run_speed = 8.f;
    float player_jump_speed = 8.f;

    auto render_size = render_->getSize();

    sf::Clock game_clock;
    const sf::Font font("fonts/OpenSans-Light.ttf");
    sf::Text ui_text(font);
    sf::Text tooltip(font);

    ui_text.setFillColor(sf::Color::White);
    ui_text.setOutlineColor(sf::Color::Black);
    ui_text.setOutlineThickness(1.0f);

    tooltip.setFillColor(sf::Color::White);
    tooltip.setOutlineColor(sf::Color::Black);
    tooltip.setOutlineThickness(1.0f);
    tooltip.setCharacterSize(20);

    int steps;
    float acc = 0.f, dt, frametime;
    auto last_time = game_clock.getElapsedTime().asMilliseconds();
    bool fade_to_black = false;
    int alpha = 255;

    PhysicsManager::init(9.8f, pixels_per_meter, meters_per_pixel, 8);
    if (debug_shapes_) {
        PhysicsManager::init_debug_draw(*render_);
    }

    load(true);

    PhysicsManager::create_physical_entities_for_tiles(render_size);

    Entity* player = EntityManager::get_entity(0);
    if (!player || player->get_serializable_type() != SET_PLAYER) {
        std::runtime_error("Level has no player entity OR the player entity is not the first entity.");
    }

    EntityManager::create_background(background_path_, 640, 480);

    auto total_time = static_cast<uint32_t>(0);
    auto next_decelerate = static_cast<uint32_t>(0);

    while (render_->isOpen())
    {
        while (const std::optional event = render_->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                render_->close();
        }

        if (!paused_) {
            total_time = game_clock.getElapsedTime().asMilliseconds();

            dt = (total_time - last_time) / 1000.f;
            frametime = std::min(maxframetime, dt);
            acc += frametime;

            auto desired_movement = sf::Vector2f();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
                desired_movement.x += player_run_speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
                desired_movement.x -= player_run_speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W) && player->get_physics()->get_grounded() > 0) {
                desired_movement.y -= player_jump_speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::R)) {
                EntityManager::score -= collected_score_;
                EntityManager::num_lives -= collected_lives_;
                EntityManager::num_keys -= collected_keys_;
                change_level(this);
            }

            PhysicsManager::queue_movement(player, desired_movement);

            for (steps = 0; acc >= fixed_dt && steps < max_steps_per_frame; ++steps, acc -= fixed_dt) {
                PhysicsManager::run(fixed_dt, total_time);
            }

            if (steps >= max_steps_per_frame) {
                acc = 0.f;
            }
        }
        else {
            fade_to_black = true;
        }

        render_->clear();

        EntityManager::update(total_time);
        EntityManager::draw(*render_);
        if (debug_shapes_) {
            PhysicsManager::draw();
        }

        FormatUI(ui_text, EntityManager::num_lives, EntityManager::num_keys, EntityManager::score);
        render_->draw(ui_text);
        if (!tooltip_text_.empty()) {
            tooltip.setString(tooltip_text_);
            tooltip.setOrigin(tooltip.getLocalBounds().getCenter());
            tooltip.setPosition({ render_size.x / 2.f, render_size.y - 20.f });
            render_->draw(tooltip);
        }

        if (fade_to_black && alpha < 255) {
            alpha += 10;
            if (alpha >= 255) {
                quit_ = true;
                alpha = 255;
            }
        }

        if (!fade_to_black && alpha > 0) {
            alpha -= 25;
        }

        if (alpha > 0) {
            sf::RectangleShape black_square({ static_cast<float>(render_size.x),
                static_cast<float>(render_size.y) });
            black_square.setFillColor(sf::Color(0, 0, 0, alpha));
            render_->draw(black_square);
        }

        render_->display();
        EntityManager::remove_marked();
        last_time = total_time;

        if (quit_) {
            break;
        }
    }

    PhysicsManager::clear_queue();
    EntityManager::remove_all();
    PhysicsManager::destroy();

    paused_ = quit_ = false;

    collected_keys_ = collected_lives_ = collected_score_ = 0;

    if (render_->isOpen()) {
        if (queued_level_) {
            EntityManager::current_level = queued_level_;
        }
        return;
    }
    EntityManager::current_level = nullptr;
}

void Level::editor_loop()
{
    std::cout << "Level editor mode is active.\n" << "Use the mouse wheel to scroll between placeable entities.\n" <<
        "Press Left click to place an entity.\n" << "Press Right click to delete an entity.\n" << 
        "Press mouse wheel to copy the entity you're currently hovering over.\n" <<
        "Press S to save the level file.\n";

    //make sure player is id 0
    Entity* player = nullptr;

    sf::Clock game_clock;
    auto last_spawned = static_cast<int32_t>(0);

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
                paused_ = true;
            }
            else if (event->is<sf::Event::FocusGained>()) {
                paused_ = false;
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

        if (paused_) {
            continue;
        }

        auto mouse = sf::Mouse::getPosition(*render_);
        auto time = game_clock.getElapsedTime().asMilliseconds();

        Entity* hovered_entity = EntityManager::get_entity(mouse.x, mouse.y, current_entity_ ? current_entity_->get_id() : -1);

        if ((time - last_spawned) > 100 && (!hovered_entity || hovered_entity->get_serializable_type() != current_blueprint_) 
            && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (current_blueprint_ == SET_PLAYER) {
                ++current_blueprint_;
            }
            if (current_entity_ && current_blueprint_ == SET_INFO) {
                std::string info;
                std::cout << "Type the text you wish this info sign to show: ";
                std::getline(std::cin, info);
                std::cout << "You've typed: " << info << '\n';
                current_entity_->add_string_data(info);
            }
            last_spawned = time;
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

        if (current_entity_ && last_blueprint != current_blueprint_) {
            if (current_entity_->get_id()) {
                EntityManager::queue_remove(current_entity_->get_id());
            }
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
            auto coeff_x = snap.x / 32;
            auto coeff_y = snap.y / 32;
            snap.x = 32 * coeff_x;
            snap.y = 32 * coeff_y;
            current_entity_->get_sprite()->set_position({ static_cast<float>(snap.x), static_cast<float>(snap.y) });

        }

        render_->clear();

        EntityManager::draw(*render_);

        render_->display();

        EntityManager::remove_marked();
    }

    EntityManager::remove_all();
}

void Level::splash_loop()
{
    auto render_size = render_->getSize();

    const sf::Font font("fonts/OpenSans-Light.ttf");

    sf::Clock game_clock;

    sf::Text game_over_text(font);
    game_over_text.setString("Game Over! Press any key.");
    game_over_text.setCharacterSize(48);
    game_over_text.setFillColor(sf::Color::White);
    game_over_text.setOutlineColor(sf::Color::Black);
    game_over_text.setOutlineThickness(1.0f);

    game_over_text.setOrigin(game_over_text.getLocalBounds().getCenter());
    game_over_text.setPosition({ render_size.x / 2.f, render_size.y - 20.f });

    game_over_text.setPosition(
        { render_size.x / 2.f, render_size.y / 2.f }
    );

    EntityManager::create_background(background_path_, 640, 480);

    while (render_->isOpen())
    {
        while (const std::optional event = render_->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                render_->close();

            if (event->is<sf::Event::KeyPressed>() && 
                game_clock.getElapsedTime().asSeconds() > 1) {
                change_level(next_level_);
            }
        }
        render_->clear();
        EntityManager::draw(*render_);
        render_->draw(game_over_text);
        render_->display();
        if (quit_ || paused_) {
            break;
        }
    }

    EntityManager::remove_all();
    EntityManager::num_lives = 3;
    EntityManager::score = EntityManager::num_keys = 0;
    quit_ = paused_ = false;
    if (render_->isOpen()) {
        if (queued_level_) {
            EntityManager::current_level = queued_level_;
        }
        return;
    }
    EntityManager::current_level = nullptr;
 }

bool Level::save()
{
    //file structure:
    //num entities (uint32)
    //for each entity:
    //  entitytype (uint8), pos_x(float), pos_y(float), extra data len (size_t), extra data, if applicable
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
        auto str = ent->get_str();
        auto len = static_cast<size_t>(0);

        //std::cout << "Saving entity number " << i << ", name: " << ent->get_name() <<
        //    ", pos: (" << pos.x << ", " << pos.y << "), serializable type: " << static_cast<int>(type) << "\n";

        fwrite(&type, sizeof(type), 1, f);
        fwrite(&pos.x, sizeof(pos.x), 1, f);
        fwrite(&pos.y, sizeof(pos.y), 1, f);
        if (str) {
            len = str->string.length();
            auto c_str = str->string.c_str();
            fwrite(&len, sizeof(len), 1, f);
            fwrite(&c_str[0], sizeof(char), len, f);
        }
        else {
            fwrite(&len, sizeof(len), 1, f);
        }
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
    std::cout << "Attempting to read " << num_entities << " entities from the level file...\n";

    uint8_t type;
    float pos_x;
    float pos_y;
    size_t len;

    for (uint32_t i = 0; i < num_entities; ++i) {
        if (fread(&type, sizeof(type), 1, f) != 1 ||
            fread(&pos_x, sizeof(pos_x), 1, f) != 1 ||
            fread(&pos_y, sizeof(pos_y), 1, f) != 1 ||
            fread(&len, sizeof(len), 1, f) != 1) {
            throw std::runtime_error("Level file is corrupted.");
        }
        //std::cout << /*"  entity: " << static_cast<int>(type) <<*/ " @ (" << pos_x << ", " << pos_y << ")\n";
        if (auto doubled = EntityManager::get_entity(pos_x, pos_y, -1)) {
            if (doubled->get_serializable_type() == type) {
                std::cout << "   > trying to place an entity on top of another entity?! (" << doubled->get_name() << ") Skipping...\n";
                continue;
            }
        }
        auto ent = EntityFromSerializableEnum(type, physics, pos_x, pos_y);

        if (len > 0) {
            std::unique_ptr<char[]> c_str(new char[len + 1]);
            c_str[len] = 0;
            if (fread(&c_str[0], sizeof(char), len, f) != len) {
                throw std::runtime_error("Level file is corrupted: Couldn't read string data for an info entity");
            }
            ent->add_string_data(c_str.get());
        }
    }

    std::cout << "Level parsed.\n";
    fclose(f);
    return true;
}

void Level::change_level(Level* level)
{
    queued_level_ = level;
    paused_ = true;
}

void Level::set_tooltip_text(const std::string& s)
{
    tooltip_text_ = s;
}

void Level::collect_key()
{
    ++collected_keys_;
}

void Level::collect_food()
{
    ++collected_score_;
}

void Level::collect_life()
{
    ++collected_lives_;
}

Level::Level(uint8_t level_state, bool debug_shapes, 
    std::string file_path, std::string background_path, sf::RenderWindow* window)
	: level_state_(level_state), debug_shapes_(debug_shapes), file_path_(file_path),
    background_path_(background_path), render_(window), current_blueprint_(SET_PLAYER), current_entity_(nullptr),
    next_level_(nullptr), fallback_level_(nullptr), paused_(false), quit_(false), queued_level_(nullptr),
    num_food_cans_(0), collected_keys_(0), collected_lives_(0), collected_score_(0)
{
}

void Level::loop()
{
    switch (level_state_) {
    case LS_EDITOR:
        editor_loop();
        break;
    case LS_NORMAL:
        regular_loop();
        break;
    case LS_SPLASH_SCREEN:
        //todo
        splash_loop();
        break;
    }
}

void Level::set_next_level(Level* level)
{
    next_level_ = level;
}

void Level::set_fallback_level(Level* level)
{
    fallback_level_ = level;
}