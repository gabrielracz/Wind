
#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>
#include "defs.h"

class Entity {
public:

    //translational
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    //rotational
    glm::vec3 rotation;
    glm::vec3 rot_velocity;
    glm::vec3 rot_acceleration;

    //std::vector<Wing> wings;
    EntityID id;

    Entity() = default;
    Entity(const glm::vec3& _position, const glm::vec3& _rotation, EntityID _id);




};


#endif
