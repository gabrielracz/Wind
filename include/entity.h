
#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>
#include "defs.h"

typedef struct Wing {
    glm::vec3 pos;
    float span;
    float chord;
    float pitch;
    float area;
    float lift_distribution;
    float dihedral;
    float stall_angle = 16.0f * M_PI/180.0f;
    float angle_of_attack;
    float flow_velocity;

    glm::vec3 lift;
    glm::vec3 drag;
    glm::vec3 net_force;
    glm::vec3 facing;
    glm::vec3 rel;
    glm::mat4 rotm = glm::mat4(1.0f);

    Wing() = default;
    Wing(const glm::vec3& pos, float span, float chord, float pitch, float dihedral, float ld = 0.4f);
    float coefficient_lift(float aoa);
    float coefficient_drag(float aoa);
    void solve_aoa(const glm::vec3& relwind);
    glm::vec3 solve_lift();
    glm::vec3 solve_drag();
    glm::vec3 solve(const glm::vec3& air);
}Wing;

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
             870.0f,   0.0f,   0.0f,
               0.0f, 2214.0f,   0.0f,
              0.0f,   0.0f, 1375.0f
            );
    // glm::mat3 inertia = glm::mat3(
    //          870.0f,   68.0f,   0.0f,
    //            68.0f, 2214.0f,   0.0f,
    //           0.0f,   0.0f, 1375.0f
    //         );

    Wing rwing;
    Wing lwing;
    Wing elevator;
    Wing rudder;

    //std::vector<Wing> wings;
    EntityID id;

    Entity() = default;
    Entity(const glm::vec3& _position, const glm::vec3& _rotation, EntityID _id);
    void update(float dt);




};


#endif
