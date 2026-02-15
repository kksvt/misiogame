#include "EntityManager.h"
#include "Level.h"

int EntityManager::create_empty_entity(const std::string& name, uint8_t type, uint8_t set_type)
{
    int i;
    for (i = 0; i < all_entities_.size(); ++i) {
        if (!all_entities_[i]) {
            break;
        }
    }
    EntityPtr_t new_ent(new Entity(i, name, type, set_type));
    if (i < all_entities_.size()) {
        all_entities_[i] = std::move(new_ent);
    }
    else {
        all_entities_.push_back(std::move(new_ent));
    }
    return i;
}

void EntityManager::Player_Think(Entity* self, uint64_t time)
{   //hack to fix an inexplicable bug
    if (time - self->ai_->last_contact > 3000) {
        self->physics_->set_grounded(1);
        self->ai_->last_contact = time;
    }
}

void EntityManager::Player_Touch(Entity* self, Entity* other, uint64_t total_time,
    b2ManifoldPoint points[2], const int num_points, b2Vec2* normal) {
    if (other->get_serializable_type() == SET_DOOR_LOCKED && num_keys > 0) {
        --num_keys;
        other->sprite_->set_animation_speed({ 100 });
        auto shape_id = other->physics_->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.maskBits = 0;
        b2Shape_SetFilter(shape_id, filter);
        return;
    }

    if (other->get_serializable_type() == SET_DOOR_EXIT) {
        num_keys = 0;
        current_level->change_level(current_level->get_next_level());
        return;
    }
}

void EntityManager::Player_Stop_Touch(Entity* self, Entity* other)
{
    if (other->get_type() == ET_PHYSICS_ONLY) {
        self->physics_->set_grounded(false);
    }
}

void EntityManager::Player_Non_Tangible_Touch(Entity* self, Entity* other) {
    if (other->get_serializable_type() == SET_ALIEN_BLUE ||
        other->get_serializable_type() == SET_ALIEN_PURPLE ||
        other->get_serializable_type() == SET_SPIKES) {
        if (num_lives > 1) {
            //restart level
            --num_lives;
            score = num_keys = 0;
            current_level->change_level(current_level);
        }
        else {
            //main menu
            num_lives = 3;
            score = num_keys = 0;
            current_level->change_level(current_level->get_fallback_level());
        }
        return;
    }

    if (other->get_type() == ET_PHYSICS_ONLY ||
        other->get_type() == ET_DOOR) {
        self->physics_->increase_contacts();
        return;
    }

    switch (other->get_serializable_type()) {
    case SET_KEY:
        EntityManager::queue_remove(other->get_id());
        ++num_keys;
        current_level->collect_key();
        return;
    case SET_INFO:
        current_level->set_tooltip_text(other->get_str()->string);
        return;
    case SET_MEDPAK:
        EntityManager::queue_remove(other->get_id());
        ++num_lives;
        current_level->collect_life();
        return;
    case SET_FOOD:
        EntityManager::queue_remove(other->get_id());
        ++score;
        current_level->collect_food();
        return;
    }
}

void EntityManager::Player_Non_Tangible_Stop_Touch(Entity* self, Entity* other) {
    if (other->get_serializable_type() == SET_INFO) {
        current_level->set_tooltip_text("");
    }
    
    if (other->get_type() == ET_PHYSICS_ONLY ||
        other->get_type() == ET_DOOR) {
        self->physics_->decrease_contacts();
    }
}

Entity* EntityManager::create_player(const std::string& sprite_path,
    int sprite_width, int sprite_height,
    bool enable_physics,
    float hitbox_width, float hitbox_height, float pos_x, float pos_y,
    const std::initializer_list<uint8_t>& num_frames,
    const std::initializer_list<uint8_t>& animation_speed)
{
    int entity_num = create_empty_entity("player", EntityType_t::ET_CHARACTER, SerializedEntityType_t::SET_PLAYER);
    Entity* player = all_entities_[entity_num].get();
    //this is kinda hacky, but im running out of time
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, player, true, hitbox_width, hitbox_height, pos_x, pos_y, true, 1.0f, 0.0f
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_PLAYER);
        filter.maskBits = UINT64_MAX & ~((1ULL << SET_ALIEN_BLUE) | (1ULL << SET_ALIEN_PURPLE) | (1ULL << SET_SPIKES) |
            (1ULL << SET_KEY) | (1ULL << SET_MEDPAK) | (1ULL << SET_FOOD) | (1ULL << SET_INFO));
        b2Shape_SetFilter(shape_id, filter);

        auto sensor = b2DefaultShapeDef();
        sensor.isSensor = true;
        sensor.enableSensorEvents = true;
        sensor.density = 1.0f;
        sensor.filter.categoryBits = (1ULL << (SET_MAX + 1));
        sensor.filter.maskBits = (1ULL << SET_ALIEN_BLUE) | (1ULL << SET_ALIEN_PURPLE) | (1ULL << SET_SPIKES) |
            (1ULL << SET_KEY) | (1ULL << SET_MEDPAK) | (1ULL << SET_FOOD) | (1ULL << SET_INFO);

        auto sensor_box = b2MakeBox(physics->get_sim_half_width(), physics->get_sim_half_height());
        auto body_id = physics->get_body_id();

        b2CreatePolygonShape(body_id, &sensor, &sensor_box);

        b2ShapeDef foot = b2DefaultShapeDef();
        foot.isSensor = true;
        foot.enableSensorEvents = true;

        foot.filter.categoryBits = (1ULL << (SET_MAX + 2));

        foot.filter.maskBits =
            UINT64_MAX &
            ~((1ULL << SET_ALIEN_BLUE) |
                (1ULL << SET_ALIEN_PURPLE) |
                (1ULL << SET_KEY) |
                (1ULL << SET_MEDPAK) |
                (1ULL << SET_FOOD) |
                (1ULL << SET_INFO));

        float hw = physics->get_sim_half_width() * 0.5f;
        float hh = physics->get_sim_half_height() * 0.1f;

        b2Vec2 offset = {
            0.0f,
            physics->get_sim_half_height() + hh + 0.01f
        };

        // Create offset box
        b2Polygon footBox = b2MakeOffsetBox(
            hw,
            hh,
            offset,
            b2MakeRot(0.0f)
        );

        b2CreatePolygonShape(body_id, &foot, &footBox);

        player->physics_ = std::move(physics);
        player->set_touch(EntityManager::Player_Touch);
        player->set_stop_touch(EntityManager::Player_Stop_Touch);
        player->set_sensor_touch(EntityManager::Player_Non_Tangible_Touch);
        player->set_sensor_stop_touch(EntityManager::Player_Non_Tangible_Stop_Touch);

        AIComponentPtr_t ai(new AIComponent_t());
        ai->desired_movement = { 0.f, 0.f };
        ai->last_contact = 0;
        player->ai_ = std::move(ai);

        player->set_process_callback(EntityManager::Player_Think);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        sprite_path, sf::IntRect({ 0, 0 }, { sprite_width, sprite_height }),
        num_frames, animation_speed, 2));
    sprite->set_position({ pos_x, pos_y });
    player->sprite_ = std::move(sprite);

    return player;
}

Entity* EntityManager::create_concrete_player(bool enable_physics, float pos_x, float pos_y)
{
    return EntityManager::create_player("sprites/misio.png", 20, 32, enable_physics, 20, 32, pos_x, pos_y, { 6, 6 }, { 100, 100 });
}

Entity* EntityManager::create_tile_unanimated(const std::string& sprite_path, int sprite_width, int sprite_height, float hitbox_width, float hitbox_height, float pos_x, float pos_y, uint8_t num_frames)
{
    int entity_num = create_empty_entity("tile", EntityType_t::ET_SPRITE_ONLY, SerializedEntityType_t::SET_TILE_FIRST);
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
        { num_frames }, { 0 }, 1));
    tile->sprite_ = std::move(sprite);

    return tile;
}

Entity* EntityManager::create_grass_tile(uint8_t grass_tile_type, float pos_x, float pos_y)
{
    Entity* tile = create_tile_unanimated("sprites/grass.png", 32, 32, 32, 32, pos_x, pos_y, 7);
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
        throw std::runtime_error("Invalid grass tile type " + grass_tile_type);
    }
    sprite->set_position({ pos_x, pos_y });
    tile->serializable_entity_type_ += grass_tile_type;
    return tile;
}

Entity* EntityManager::create_background(const std::string& sprite_path, int width, int height)
{
    int entity_num = create_empty_entity("background", EntityType_t::ET_SPRITE_ONLY, SerializedEntityType_t::SET_MAX);
    Entity* bg = all_entities_[entity_num].get();
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        sprite_path, sf::IntRect({ 0, 0 }, { width, height }),
        { 1 }, { 0 }, 0));
    bg->sprite_ = std::move(sprite);
    return bg;
}

Entity* EntityManager::create_physical_box(float hitbox_width, float hitbox_height, float pos_x, float pos_y)
{
    int entity_num = create_empty_entity("physics", EntityType_t::ET_PHYSICS_ONLY, SerializedEntityType_t::SET_MAX);
    Entity* phys = all_entities_[entity_num].get();
    PhysicsComponentPtr_t physics(new PhysicsComponent_t(
        PhysicsManager::world_id, phys, false, hitbox_width, hitbox_height, pos_x, pos_y, true
    ));
    auto shape_id = physics->get_shape_id();
    auto filter = b2Shape_GetFilter(shape_id);
    filter.categoryBits = (1ULL << SET_MAX);
    filter.maskBits = UINT64_MAX;
    b2Shape_SetFilter(shape_id, filter);
    phys->physics_ = std::move(physics);
    return phys;
}

Entity* EntityManager::create_trigger_hurt(float hitbox_width, float hitbox_height, float pos_x, float pos_y)
{
    int entity_num = create_empty_entity("hurt", EntityType_t::ET_PHYSICS_ONLY, SerializedEntityType_t::SET_SPIKES);
    Entity* phys = all_entities_[entity_num].get();
    PhysicsComponentPtr_t physics(new PhysicsComponent_t(
        PhysicsManager::world_id, phys, false, hitbox_width, hitbox_height, pos_x, pos_y, true
    ));
    auto shape_id = physics->get_shape_id();
    auto filter = b2Shape_GetFilter(shape_id);
    filter.categoryBits = (1ULL << SET_SPIKES);
    filter.maskBits = UINT64_MAX;
    b2Shape_SetFilter(shape_id, filter);
    phys->physics_ = std::move(physics);
    return phys;
}

//std::function<void(Entity*, uint64_t)>
void EntityManager::Purple_Baddie_AI(Entity* self, uint64_t time) {
    //just walk and bounce off walls. if you fall, you fall
    if (time - self->ai_->last_contact > 10000) {
        self->ai_->desired_movement.x *= -1.f;
        self->ai_->last_contact = time;
    }
    PhysicsManager::queue_movement(self, self->ai_->desired_movement);
}

void EntityManager::Purple_Baddie_Touch(Entity* self, Entity* other, uint64_t total_time,
    b2ManifoldPoint points[2], const int num_points, b2Vec2* normal) {
    if (other->get_serializable_type() == SET_SPIKES) {
        EntityManager::queue_remove(self->get_id());
        return;
    }

    //if (other->get_type() == ET_PHYSICS_ONLY) {
    if (std::abs(normal->x) == 1.0f) {
        //time to switch directions
        self->ai_->desired_movement.x *= -1.f;
        self->ai_->last_contact = total_time; //this is bad, but i want to make sure that enemies dont get stuck
    }
    //}
}

Entity* EntityManager::create_baddie_one(bool enable_physics, float pos_x, float pos_y)
{
    int entity_num = create_empty_entity("baddie1", EntityType_t::ET_CHARACTER, SerializedEntityType_t::SET_ALIEN_PURPLE);
    Entity* alien = all_entities_[entity_num].get();
    //this is kinda hacky, but im running out of time
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, alien, true, 20, 32, pos_x, pos_y, true, 1.0f, 0.0f
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_ALIEN_PURPLE);
        filter.maskBits = UINT64_MAX & ~((1ULL << SET_PLAYER) | (1ULL << SET_KEY) | (1ULL << SET_FOOD) | (1ULL << SET_MEDPAK));
        b2Shape_SetFilter(shape_id, filter);

        alien->physics_ = std::move(physics);
        alien->set_process_callback(EntityManager::Purple_Baddie_AI);
        alien->set_touch(EntityManager::Purple_Baddie_Touch);

        AIComponentPtr_t ai(new AIComponent_t());
        ai->desired_movement = { 1.0f, 0.f };
        ai->last_contact = 0.f;
        alien->ai_ = std::move(ai);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/evilalien.png", sf::IntRect({ 0, 0 }, { 20, 32 }),
        { 6 }, { 100 }, 2));
    sprite->set_position({ pos_x, pos_y });
    alien->sprite_ = std::move(sprite);

    return alien;
}

void Locked_Door_Process(Entity* self, uint64_t time) {
    auto sprite = self->get_sprite();
    if (sprite->get_animation_speed() > 0 &&
        sprite->get_frame() > 10) {
        sprite->set_animation_speed({ 0 });
        EntityManager::queue_remove(self->get_id());
    }
}

Entity* EntityManager::create_door_locked(bool enable_physics, float pos_x, float pos_y) {
    int entity_num = create_empty_entity("locked_door", EntityType_t::ET_DOOR, SerializedEntityType_t::SET_DOOR_LOCKED);
    Entity* door = all_entities_[entity_num].get();
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, door, false, 32.f, 64.f, pos_x, pos_y, true
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_DOOR_LOCKED);
        filter.maskBits = UINT64_MAX;
        b2Shape_SetFilter(shape_id, filter);

        door->physics_ = std::move(physics);
        door->set_process_callback(Locked_Door_Process);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/door.png", sf::IntRect({ 0, 0 }, { 32, 64 }),
        { 12 }, { 0 }, 2));
    sprite->set_position({ pos_x, pos_y });
    door->sprite_ = std::move(sprite);

    return door;
}

Entity* EntityManager::create_door_exit(bool enable_physics, float pos_x, float pos_y) {
    int entity_num = create_empty_entity("exit_door", EntityType_t::ET_DOOR, SerializedEntityType_t::SET_DOOR_EXIT);
    Entity* door = all_entities_[entity_num].get();
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, door, false, 32.f, 64.f, pos_x, pos_y, true
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_DOOR_EXIT);
        filter.maskBits = UINT64_MAX;
        b2Shape_SetFilter(shape_id, filter);

        door->physics_ = std::move(physics);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/exit.png", sf::IntRect({ 0, 0 }, { 32, 64 }),
        { 1 }, { 0 }, 2));
    sprite->set_position({ pos_x, pos_y });
    door->sprite_ = std::move(sprite);

    return door;
}

Entity* EntityManager::create_key(bool enable_physics, float pos_x, float pos_y) {
    int entity_num = create_empty_entity("key", EntityType_t::ET_COLLECTIBLE, SerializedEntityType_t::SET_KEY);
    Entity* key = all_entities_[entity_num].get();
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, key, false, 32.f, 32, pos_x, pos_y, true, 1.0f, 0.0f
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_KEY);
        filter.maskBits = UINT64_MAX & ~(1ULL << SET_PLAYER);
        b2Shape_SetFilter(shape_id, filter);

        key->physics_ = std::move(physics);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/collectibles.png", sf::IntRect({ 0, 0 }, { 32, 32 }),
        { 12 }, { 0 }, 2));
    sprite->set_position({ pos_x, pos_y });
    key->sprite_ = std::move(sprite);

    return key;
}

Entity* EntityManager::create_spikes(bool enable_physics, float pos_x, float pos_y) {
    int entity_num = create_empty_entity("spikes", EntityType_t::ET_OBSTACLE, SerializedEntityType_t::SET_SPIKES);
    Entity* spikes = all_entities_[entity_num].get();
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, spikes, false, 32, 32, pos_x, pos_y, true
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_SPIKES);
        filter.maskBits = UINT64_MAX & ~(1ULL << SET_PLAYER);
        b2Shape_SetFilter(shape_id, filter);

        spikes->physics_ = std::move(physics);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/spikes.png", sf::IntRect({ 0, 0 }, { 32, 32 }),
        { 1 }, { 0 }, 1));
    sprite->set_position({ pos_x, pos_y });
    spikes->sprite_ = std::move(sprite);

    return spikes;
}

Entity* EntityManager::create_medpak(bool enable_physics, float pos_x, float pos_y) {
    int entity_num = create_empty_entity("medpak", EntityType_t::ET_COLLECTIBLE, SerializedEntityType_t::SET_MEDPAK);
    Entity* medpak = all_entities_[entity_num].get();
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, medpak, false, 32, 32, pos_x, pos_y, true
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_MEDPAK);
        filter.maskBits = UINT64_MAX & ~(1ULL << SET_PLAYER);
        b2Shape_SetFilter(shape_id, filter);

        medpak->physics_ = std::move(physics);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/collectibles.png", sf::IntRect({ 64, 0 }, { 32, 32 }),
        { 1 }, { 0 }, 1));
    sprite->set_position({ pos_x, pos_y });
    medpak->sprite_ = std::move(sprite);

    return medpak;
}

Entity* EntityManager::create_info_sign(bool enable_physics, float pos_x, float pos_y) {
    int entity_num = create_empty_entity("info", EntityType_t::ET_INFO, SerializedEntityType_t::SET_INFO);
    Entity* info = all_entities_[entity_num].get();
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, info, false, 32, 32, pos_x, pos_y, true
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_INFO);
        filter.maskBits = UINT64_MAX & ~(1ULL << SET_PLAYER);
        b2Shape_SetFilter(shape_id, filter);

        info->physics_ = std::move(physics);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/info.png", sf::IntRect({ 0, 0 }, { 32, 32 }),
        { 1 }, { 0 }, 1));
    sprite->set_position({ pos_x, pos_y });
    info->sprite_ = std::move(sprite);

    return info;
}

Entity* EntityManager::create_food(bool enable_physics, float pos_x, float pos_y) {
    int entity_num = create_empty_entity("food", EntityType_t::ET_COLLECTIBLE, SerializedEntityType_t::SET_FOOD);
    Entity* food = all_entities_[entity_num].get();
    if (enable_physics) {
        PhysicsComponentPtr_t physics(new PhysicsComponent_t(
            PhysicsManager::world_id, food, false, 32, 32, pos_x, pos_y, true
        ));

        auto shape_id = physics->get_shape_id();
        auto filter = b2Shape_GetFilter(shape_id);
        filter.categoryBits = (1ULL << SET_FOOD);
        filter.maskBits = UINT64_MAX & ~(1ULL << SET_PLAYER);
        b2Shape_SetFilter(shape_id, filter);

        food->physics_ = std::move(physics);
    }
    SpriteComponentPtr_t sprite(new SpriteComponent_t(
        "sprites/collectibles.png", sf::IntRect({ 32, 0 }, { 32, 32 }),
        { 1 }, { 0 }, 1));
    sprite->set_position({ pos_x, pos_y });
    food->sprite_ = std::move(sprite);

    return food;
}