#include "PhysicsComponent.h"
#include "Entity.h"

PhysicsComponent_t::PhysicsComponent_t(
	const b2WorldId& world_id, Entity* ent,
    bool is_dynamic, float width, float height, 
	float pos_x, float pos_y, float density, float friction) : last_moved_(0)
{
    half_width_ = width / 2.f;
    half_height_ = height / 2.f;

    body_def_ = b2DefaultBodyDef();
    body_def_.type = is_dynamic ? b2_dynamicBody : b2_staticBody;
    body_def_.position = { 
        (pos_x + half_width_) * PhysicsManager::meters_per_pixel, 
        (pos_y + half_height_) * PhysicsManager::meters_per_pixel
    };

    body_def_.fixedRotation = true;

    body_def_.userData = ent; //ent comes from a unique ptr, but this is fine due to how the entity removal system is implemented

    body_id_ = b2CreateBody(world_id, &body_def_);

    body_polygon_ = b2MakeBox(
        half_width_ * PhysicsManager::meters_per_pixel,
        half_height_ * PhysicsManager::meters_per_pixel);

    body_shape_def_ = b2DefaultShapeDef();
    body_shape_def_.density = density;
    body_shape_def_.material.friction = friction;

    b2CreatePolygonShape(body_id_, &body_shape_def_, &body_polygon_);
}

PhysicsComponent_t::~PhysicsComponent_t()
{
    b2DestroyBody(body_id_);
}

sf::Vector2f PhysicsComponent_t::get_position()
{
    auto box_pos = b2Body_GetPosition(body_id_);
    return sf::Vector2f(
        (box_pos.x * PhysicsManager::pixels_per_meter) - half_width_,
        (box_pos.y * PhysicsManager::pixels_per_meter) - half_height_);
}

void PhysicsComponent_t::apply_force(const sf::Vector2f& force)
{

    //b2Body_ApplyLinearImpulseToCenter(_body_id, { force.x * PhysicsManager::meters_per_pixel, force.y * PhysicsManager::meters_per_pixel }, true);
    b2Body_ApplyForce(
        body_id_, 
        {   force.x /** PhysicsManager::meters_per_pixel*/, 
            force.y /** PhysicsManager::meters_per_pixel*/ 
        }, 
        b2Body_GetWorldCenterOfMass(body_id_), 
        true
    );
}

void PhysicsComponent_t::move(const sf::Vector2f& dir, float run_speed, float jump_speed)
{
    auto v = b2Body_GetLinearVelocity(body_id_);
   
    auto target_v = b2Vec2{ dir.x * run_speed, v.y };

    if (dir.y && !v.y) {
        target_v.y = dir.y * jump_speed;
    }

    b2Body_SetLinearVelocity(body_id_, target_v);
}

void PhysicsComponent_t::decelerate(uint64_t time)
{
    /*auto v = b2Body_GetLinearVelocity(body_id_);
    float mass = b2Body_GetMass(body_id_);
    float impulse = mass * (0.f - v.x) * dt;
    b2Body_ApplyLinearImpulseToCenter(body_id_, { impulse, 0.f }, true);*/
    auto v = b2Body_GetLinearVelocity(body_id_);
    v.x *= -0.5f;
    v.y = 0.f;
    b2Body_ApplyLinearImpulseToCenter(body_id_, v, true);
    //b2Body_SetLinearVelocity(body_id_, v);
}