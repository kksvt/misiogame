#include "EntityManager.h"

std::vector<EntityPtr_t> EntityManager::all_entities_;
std::set<int> EntityManager::remove_entities_;
Level* EntityManager::current_level;
int16_t EntityManager::num_lives;
int16_t EntityManager::score;
int16_t EntityManager::num_keys;

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
        if (pos_x >= pos_lower.x && pos_x < pos_upper.x &&
            pos_y >= pos_lower.y && pos_y < pos_upper.y) {
            return it.get();
        }
    }
    return nullptr;
}

void EntityManager::remove_marked()
{
    for (auto it : remove_entities_) {
        PhysicsManager::remove_movement(all_entities_[it].get());
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
    remove_entities_.clear();
}

uint32_t EntityManager::get_num_active_entities()
{
    uint32_t num = 0;
    for (auto& it : all_entities_) {
        if (it) {
            ++num;
        }
    }
    return num;
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
        if (!it) {
            continue;
        }
        it->update(time);
    }
}