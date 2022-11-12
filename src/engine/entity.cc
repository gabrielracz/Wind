#include "entity.h"

Entity::Entity(const glm::vec3 &_position, const glm::vec3 &_rotation, EntityID _id)
    : position(_position),
    rotation(_rotation),
    id(_id)
{}