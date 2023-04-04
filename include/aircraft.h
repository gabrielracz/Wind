
#ifndef ENTITY_H
#define ENTITY_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
//#include <math.h>
#include <cmath>
#include "defs.h"

#define LBS_TO_KG 0.453592f

enum class Pitch {
    Up,
    Down,
    Neutral
};

typedef struct Wing {
    glm::vec3 pos = glm::vec3(0.0f);
    float span = 0.0f;
    float chord = 0.0f;
    float pitch = 0.0f;
    float resting_pitch = 0.0f;
    float area = 0.0f;
    float lift_distribution = 0.0f;
    float dihedral = 0.0f;
    float stall_angle = 16.0f * M_PI/180.0f;
    float angle_of_attack = 0.0f;
    float flow_velocity = 0.0f;
    bool stalled = false;

    glm::vec3 lift = glm::vec3(0.0f);
    glm::vec3 drag = glm::vec3(0.0f);
    glm::vec3 net_force = glm::vec3(0.0f);
    glm::vec3 facing = glm::vec3(0.0);
    glm::vec3 rel = glm::vec3(0.0);
    glm::mat4 rotm = glm::mat4(1.0f);
    glm::vec3 center_of_pressure = glm::vec3(0.0);

    Wing() = default;
    Wing(const glm::vec3& pos, float span, float chord, float pitch, float dihedral, float ld = 0.5f);
    float coefficient_lift(float aoa);
    float coefficient_drag(float aoa);
    void solve_aoa(const glm::vec3& relwind, const glm::vec3& rvelocity);
    glm::vec3 solve_lift();
    glm::vec3 solve_drag();
    glm::vec3 solve(const glm::vec3& air, const glm::vec3& rvelocity);
    void update_rotation();
    void change_pitch(Pitch p);


}Wing;

class Aircraft {
public:

    float elapsed = 0.0f;
    //translational
    glm::vec3 position;
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);

    glm::vec3 thrust = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::mat4 rotm = glm::mat4(1.0f);
    glm::vec3 fuselage_drag = glm::vec3(0.0f);

    //rotational
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 rot_velocity = glm::vec3(0.0f);
    glm::vec3 rot_acceleration = glm::vec3(0.0f);

   //  https://jsbsim.sourceforge.net/MassProps.html
   //  (Iyy -> Ixx)    (Izz -> Iyy)    (Ixx -> Izz)

    // Sailplane
    // float mass = 350.44f;
    // glm::mat3 inertia = glm::mat3(
    //          870.0f,   0.0f,   0.0f,
    //            0.0f, 2214.0f,   0.0f,
    //           0.0f,   0.0f, 1375.0f
    //         );


    float mass = 1700 * LBS_TO_KG;
    // pitch   yaw  roll
    glm::mat3 inertia = glm::mat3(
             546.0f,   0.0f,   0.0f,
               0.0f, 967.0f,   0.0f,
               0.0f,     0.0,    848.0f
    ) * LBS_TO_KG;
    //C172
    // glm::mat3 inertia = glm::mat3(
    //          1346.0f,   0.0f,   0.0f,
    //            0.0f, 1967.0f,   0.0f,
    //            0.0f,     0.0,    948.0f
    // ) * LBS_TO_KG;

    bool throttle = false;


    Wing rwing;
    Wing lwing;
    Wing elevator;
    Wing rudder;

    //std::vector<Wing> wings;
    EntityID id;

    Aircraft() = default;
    Aircraft(const glm::vec3& _position, const glm::vec3& _rotation, EntityID _id);
    void update(float dt);




};


#endif
