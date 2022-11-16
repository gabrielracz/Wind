
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
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::mat4 rotm = glm::mat4(1.0f);

    //rotational
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 rot_velocity = glm::vec3(0.0f);
    glm::vec3 rot_acceleration = glm::vec3(0.0f);

   //https://jsbsim.sourceforge.net/MassProps.html
    float mass = 396.44f;
    glm::mat3 inertia = glm::mat3(
            1375.0f,   0.0f,   68.0f,
               0.0f, 870.0f,    0.0f,
              68.0f,   0.0f, 2214.0f
            );
    float chord = 1.0f;
    float span  = 14.0f;

    //std::vector<Wing> wings;
    EntityID id;

    Entity() = default;
    Entity(const glm::vec3& _position, const glm::vec3& _rotation, EntityID _id);
    void update(float dt);




};


#endif
