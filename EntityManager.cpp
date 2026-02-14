#include "EntityManager.h"
#include <iostream>
#include <SFML/Graphics.hpp>

std::vector<EntityPtr_t> EntityManager::all_entities_;
std::set<int> EntityManager::remove_entities_;

int EntityManager::create_empty_entity(const std::string& name, uint8_t type)
{
    int i;
    for (i = 0; i < all_entities_.size(); ++i) {
        if (!all_entities_[i]) {
            break;
        }
    }
    EntityPtr_t new_ent(new Entity(i, name, type));
    if (i < all_entities_.size()) {
        all_entities_[i] = std::move(new_ent);
    }
    else {
        all_entities_.push_back(std::move(new_ent));
    }
    return i;
}

Entity* EntityManager::create_player(const std::string& sprite_path,
    int sprite_width, int sprite_height,
    bool enable_physics,
    float hitbox_width, float hitbox_height, float pos_x, float pos_y,
    const std::initializer_list<uint8_t>& num_frames,
    const std::initializer_list<uint8_t>& animation_speed)
{
    int entity_num = create_empty_entity("player", EntityType_t::ET_CHARACTER);
    Entity* player = all_entities_[entity_num].get();
    //this is kinda hacky, but im running out of time
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, player, true, hitbox_width, hitbox_height, pos_x, pos_y, 1.0f, 0.0f
        ));
        player->physics_ = std::move(physics);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        sprite_path, sf::IntRect({ 0, 0 }, {sprite_width, sprite_height}), 
        num_frames, animation_speed, 2));
    player->sprite_ = std::move(sprite);

    std::cout << "Player character created as entity number " << entity_num << '\n';

    return player;
}

Entity* EntityManager::create_concrete_player(bool enable_physics, float pos_x, float pos_y)
{
    return EntityManager::create_player("misio.png", 20, 35, enable_physics, 20, 35, pos_x, pos_y, { 6, 6 }, { 100, 100 });
}

Entity* EntityManager::create_tile_unanimated(const std::string& sprite_path, int sprite_width, int sprite_height, float hitbox_width, float hitbox_height, float pos_x, float pos_y, uint8_t num_frames)
{
    int entity_num = create_empty_entity("tile", EntityType_t::ET_SPRITE_ONLY);
    Entity* tile = all_entities_[entity_num].get();
    //we actually cannot let every single individual tile be a separate physics object
    /*
    PhysicsComponentPtr_t physics(new PhysicsComponent_t(
        PhysicsManager::world_id, tile, false, hitbox_width, hitbox_height, pos_x, pos_y
    ));
    tile->physics_ = std::move(physics);
    }*/
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        sprite_path, sf::IntRect({ 0, 0 }, { sprite_width, sprite_height }),
        { num_frames }, {0}, 1));
    tile->sprite_ = std::move(sprite);

    std::cout << "Tile created as entity number " << entity_num << '\n';

    return tile;
}

Entity* EntityManager::create_grass_tile(uint8_t grass_tile_type, float pos_x, float pos_y)
{
    Entity* tile = create_tile_unanimated("grass.png", 32, 32, 32, 32, pos_x, pos_y, 7);
    auto sprite = tile->get_sprite();
    switch (grass_tile_type) {
    case GT_DIRT:
        sprite->set_square(2);
        break;
    case GT_SINGLE_ROW_LEFT:
        sprite->set_square(3);
        break;
    case GT_SINGLE_ROW_MIDDLE:
        sprite->set_square(4);
        break;
    case GT_SINGLE_ROW_RIGHT:
        sprite->set_square(3);
        sprite->set_flipped(true, false);
        break;
    case GT_MULTI_LEFT_BOTTOM:
        sprite->set_square(0);
        sprite->set_flipped(false, true);
        break;
    case GT_MULTI_RIGHT_BOTTOM:
        sprite->set_square(0);
        sprite->set_flipped(true, true);
        break;
    case GT_MULTI_MIDDLE_BOTTOM:
        sprite->set_square(1);
        sprite->set_flipped(false, true);
        break;
    case GT_MULTI_LEFT_TOP:
        sprite->set_square(0);
        break;
    case GT_MULTI_RIGHT_TOP:
        sprite->set_square(0);
        sprite->set_flipped(true, false);
        break;
    case GT_MULTI_MIDDLE_TOP:
        sprite->set_square(1);
        break;
    case GT_MULTI_LEFT:
        sprite->set_square(6);
        break;
    case GT_MULTI_RIGHT:
        sprite->set_square(6);
        sprite->set_flipped(true, false);
        break;
    case GT_CORNER_TOP_LEFT:
        sprite->set_square(5);
        break;
    case GT_CORNER_TOP_RIGHT:
        sprite->set_square(5);
        sprite->set_flipped(true, false);
        break;
    case GT_CORNER_BOTTOM_LEFT:
        sprite->set_square(5);
        sprite->set_flipped(false, true);
        break;
    case GT_CORNER_BOTTOM_RIGHT:
        sprite->set_square(5);
        sprite->set_flipped(true, true);
        break;
    default:
        std::cerr << "Invalid grass tile type " << grass_tile_type << '\n';
    }
    return tile;
}

Entity* EntityManager::create_background(const std::string& sprite_path, int width, int height)
{
    int entity_num = create_empty_entity("background", EntityType_t::ET_SPRITE_ONLY);
    Entity* bg = all_entities_[entity_num].get();
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        sprite_path, sf::IntRect({ 0, 0 }, { width, height }),
        { 1 }, { 0 }, 0));
    bg->sprite_ = std::move(sprite);
    std::cout << "Background created as entity number " << entity_num << '\n';
    return bg;
}

void EntityManager::queue_remove(int entity_num)
{
    remove_entities_.insert(entity_num);
}

Entity* EntityManager::get_entity(int entity_num)
{
    if (entity_num >= 0 && entity_num < all_entities_.size()) {
        return all_entities_[entity_num].get();
    }
    return nullptr;
}

Entity* EntityManager::get_entity(float pos_x, float pos_y, int ignore)
{
    for (auto& it : all_entities_) {
        if (!it || it->id_ == ignore) {
            continue;
        }
        sf::Vector2f pos_lower;
        sf::Vector2f size;
        if (it->physics_) {
            pos_lower = it->physics_->get_position();
            size = it->physics_->get_hitbox();
        }
        else if (it->sprite_) {
            pos_lower = it->sprite_->get_position();
            auto sizei = it->sprite_->get_size();
            size.x = static_cast<float>(sizei.x);
            size.y = static_cast<float>(sizei.y);
        }
        else {
            continue;
        }
        sf::Vector2f pos_upper = pos_lower + size;
        if (pos_x >= pos_lower.x && pos_x <= pos_upper.x &&
            pos_y >= pos_lower.y && pos_y <= pos_upper.y) {
            return it.get();
        }
    }
    return nullptr;
}

void EntityManager::remove_marked()
{
    for (auto it : remove_entities_) {
        all_entities_[it].reset();
    }
    remove_entities_.clear();
}

void EntityManager::remove_all()
{
    for (auto &it : all_entities_) {
        it.reset();
    }
    all_entities_.clear();
}

void EntityManager::draw(sf::RenderWindow& window)
{
    std::vector<SpriteComponent_t*> sprites;
    sprites.reserve(all_entities_.size());
    for (auto& it : all_entities_) {
        if (it && it->sprite_ && //it can be null here if an entity gets removed and a new entity doesnt replace it
            it->sprite_->valid_sprite()) {
            sprites.push_back(it->get_sprite());
        }
    }

    std::sort(sprites.begin(), sprites.end(), [](const SpriteComponent_t* a, const SpriteComponent_t* b) {
        return a->get_priority() < b->get_priority();
        });

    for (auto& it : sprites) {
        window.draw(it->get_sprite());
    }
}

void EntityManager::update(uint64_t time) {
    for (auto& it : all_entities_) {
        it->update(time);
    }
}