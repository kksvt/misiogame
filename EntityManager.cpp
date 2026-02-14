#include "EntityManager.h"
#include <iostream>
#include <SFML/Graphics.hpp>

std::vector<EntityPtr_t> EntityManager::all_entities_;
std::set<int> EntityManager::remove_entities_;

int EntityManager::create_empty_entity(const std::string& name)
{
    int i;
    for (i = 0; i < all_entities_.size(); ++i) {
        if (!all_entities_[i]) {
            break;
        }
    }
    EntityPtr_t new_ent(new Entity(i, name));
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
    float hitbox_width, float hitbox_height, float pos_x, float pos_y,
    const std::initializer_list<uint8_t>& num_frames,
    const std::initializer_list<uint8_t>& animation_speed)
{
    int entity_num = create_empty_entity("player");
    Entity* player = all_entities_[entity_num].get();
    PhysicsComponentPtr_t physics(new PhysicsComponent_t(
        PhysicsManager::world_id, player, true, hitbox_width, hitbox_height, pos_x, pos_y
    ));
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        sprite_path, sf::IntRect({ 0, 0 }, {sprite_width, sprite_height}), 
        num_frames, animation_speed ));
    player->physics_ = std::move(physics);
    player->sprite_ = std::move(sprite);

    std::cout << "Player character created as entity number " << entity_num << '\n';

    return player;
}

Entity* EntityManager::create_tile_unanimated(const std::string& sprite_path, int sprite_width, int sprite_height, float hitbox_width, float hitbox_height, float pos_x, float pos_y, uint8_t num_frames)
{
    int entity_num = create_empty_entity("tile");
    Entity* tile = all_entities_[entity_num].get();
    PhysicsComponentPtr_t physics(new PhysicsComponent_t(
        PhysicsManager::world_id, tile, false, hitbox_width, hitbox_height, pos_x, pos_y
    ));
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        sprite_path, sf::IntRect({ 0, 0 }, { sprite_width, sprite_height }),
        { num_frames }, {0}));
    tile->physics_ = std::move(physics);
    tile->sprite_ = std::move(sprite);

    std::cout << "Tile created as entity number " << entity_num << '\n';

    return tile;
}

Entity* EntityManager::create_grass_tile(uint8_t grass_tile_type, float pos_x, float pos_y)
{
    Entity* tile = create_tile_unanimated("grass.png", 32, 32, 32, 32, pos_x, pos_y, 6);
    auto sprite = tile->get_sprite();
    switch (grass_tile_type) {
    case GT_DIRT:
        break;
    case GT_SINGLE_ROW_LEFT:
        break;
    case GT_SINGLE_ROW_MIDDLE:
        break;
    case GT_SINGLE_ROW_RIGHT:
        break;
    case GT_MULTI_LEFT_BOTTOM:
        break;
    case GT_MULTI_RIGHT_BOTTOM:
        break;
    case GT_MULTI_MIDDLE_BOTTOM:
        break;
    case GT_MULTI_LEFT_TOP:
        break;
    case GT_MULTI_RIGHT_TOP:
        break;
    case GT_MULTI_MIDDLE_TOP:
        break;
    case GT_MULTI_LEFT:
        break;
    case GT_MULTI_RIGHT:
        break;
    case GT_CORNER_TOP_LEFT:
        break;
    case GT_CORNER_TOP_RIGHT:
        break;
    case GT_CORNER_BOTTOM_LEFT:
        break;
    case GT_CORNER_BOTTOM_RIGHT:
        break;
    default:
        std::cerr << "Invalid grass tile type " << grass_tile_type << '\n';
    }
    return tile;
}

void EntityManager::queue_remove(int entity_num)
{
    remove_entities_.insert(entity_num);
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
    for (auto& it : all_entities_) {
        if (it->sprite_ && it->sprite_->valid_sprite()) {
            window.draw(it->sprite_->get_sprite());
        }
    }
}

void EntityManager::update(uint64_t time) {
    for (auto& it : all_entities_) {
        it->update(time);
    }
}