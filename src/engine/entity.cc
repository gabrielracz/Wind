#include "entity.h"
#include <glm/gtx/vector_angle.hpp>

Entity::Entity(const glm::vec3 &_position, const glm::vec3 &_rotation, EntityID _id)
    : position(_position),
    rotation(_rotation),
    id(_id)
{}

void Entity::update(float dt) {


    //integrate accel, use moment of intertia
    rot_velocity += glm::inverse(inertia) * glm::vec3(rot_acceleration.x, rot_acceleration.y, -rot_acceleration.z) * dt;
    rot_velocity *= 0.99;

    float theta = glm::length(rot_velocity);
    if(theta > 0.0f) {
        glm::mat4 frame_rotation = glm::rotate(glm::mat4(1.0f), glm::length(rot_velocity), rot_velocity);
        rotm = rotm * frame_rotation;
    }

    rot_acceleration.x = 0;
    rot_acceleration.y = 0;
    rot_acceleration.z = 0;
}