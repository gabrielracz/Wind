#include "entity.h"
#include <GLFW/glfw3.h>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/matrix.hpp>
#include <ios>
#include <iostream>
#include <iomanip>
#include <glm/gtx/string_cast.hpp>

const float AIR_DENSITY = 1.29f;

Wing::Wing(const glm::vec3 &pos, float span, float chord, float pitch, float dihedral, float ld)
    : pos(pos), span(span), chord(chord), area(abs((int)span)*chord), lift_distribution(ld), dihedral(dihedral)
{
    center_of_pressure = glm::vec3(span*lift_distribution, span*sin(dihedral), chord*0.25);
    glm::vec3 rot(pitch, 0.0f, dihedral);
    if(glm::length(rot) > 0.0f)
        rotm = glm::rotate(glm::mat4(1.0f), glm::length(rot), rot);
}

float Wing::coefficient_lift(float aoa) {
    float max_cl = 1.2;
    if(aoa <= stall_angle && aoa >= -stall_angle) {
        return (max_cl / stall_angle) * aoa;
    } else {
        return sin(2*aoa);
    }
}

float Wing::coefficient_drag(float aoa) {
    return 1 - cos(2*aoa);
}

void Wing::solve_aoa(const glm::vec3& air) {
    glm::mat4 plane_to_wing = glm::inverse(rotm);
    glm::vec3 relwind = plane_to_wing * glm::vec4(air, 1.0f);
    glm::vec3 relwind_yz(0.0f, relwind.y, relwind.z);
    flow_velocity = glm::length(relwind_yz);
    rel = relwind;
    glm::vec3 wing_chord = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    angle_of_attack = atan2(glm::dot(glm::cross(wing_chord, relwind_yz), glm::vec3(-1.0f, 0.0f, 0.0f)),
                                     glm::dot(wing_chord, relwind_yz));
}

glm::vec3 Wing::solve_lift() {
    float cl = coefficient_lift(angle_of_attack);
    float lift = cl * (AIR_DENSITY * flow_velocity*flow_velocity)/2 * area;
    glm::vec3 lift_vector = glm::normalize(glm::cross(glm::vec3(-1.0f, 0.0f, 0.0f), rel)) * lift;
    return lift_vector;
}

glm::vec3 Wing::solve_drag() {
    float cd = coefficient_drag(angle_of_attack);
    float drag = cd * (AIR_DENSITY * flow_velocity*flow_velocity)/2 * area;
    glm::vec3 drag_vector = glm::normalize(rel) * drag;
    return drag_vector;
}

glm::vec3 Wing::solve(const glm::vec3& air) {
    solve_aoa(air);
    lift = solve_lift();
    drag = solve_drag();
    net_force = rotm * glm::vec4(lift+drag, 1.0f);
    return net_force;
}



Entity::Entity(const glm::vec3 &_position, const glm::vec3 &_rotation, EntityID _id)
    : position(_position),
    rotation(_rotation),
    id(_id),
    rwing(glm::vec3(0.0f, 0.0f, 0.4f), 7, 1, 0.0, 0.13),
    lwing(glm::vec3(0.0f, 0.0f, 0.4f), -7, 1, 0.0, -0.13), //neg dihedral
	elevator(glm::vec3(0.0f, 1.25f, 4.5f), 1, 0.5, -0.01f, 0.0f, 0.0f),
	rudder(glm::vec3(0.0f, 0.0f, 4.5f), 1, 1.0f, 0.0f, M_PI_2)
{}

void Entity::update(float dt) {

    // velocity = glm::vec3(0.0f);
    // velocity = glm::inverse(rotm) * glm::vec4(0.0f, 0.0f, 30.0f, 0.0f);
    glm::vec3 air = -velocity;

    lwing.solve(air);
    rwing.solve(air);
    elevator.solve(air);
    rudder.solve(air);

    glm::vec3 torque;
    torque += glm::cross(rwing.pos + rwing.center_of_pressure, rwing.net_force);
    torque += glm::cross(lwing.pos + lwing.center_of_pressure, lwing.net_force);
    torque += glm::cross(elevator.pos, elevator.net_force);
    torque += glm::cross(rudder.pos, rudder.net_force);


    //integrate
    rot_acceleration += glm::inverse(inertia) * torque;

    rot_velocity += glm::inverse(inertia) * glm::vec3(rot_acceleration.x, rot_acceleration.y, -rot_acceleration.z) * dt;
    rot_velocity *= 0.99; //some damping

    float theta = glm::length(rot_velocity);
    if(theta > 0.0f) {
        glm::mat4 frame_rotation = glm::rotate(glm::mat4(1.0f), glm::length(rot_velocity), rot_velocity);
        rotm = rotm * frame_rotation;
    }

    rot_acceleration.x = 0;
    rot_acceleration.y = 0;
    rot_acceleration.z = 0;

    glm::vec3 translational_force(0.0f);

    translational_force += rwing.net_force;
    translational_force += lwing.net_force;
    translational_force += elevator.net_force;
    translational_force += rudder.net_force;

    acceleration += translational_force / mass;
    acceleration += thrust / mass;
    velocity += acceleration * dt;
    std::cout << glm::to_string(translational_force) << std::endl;
    acceleration =  glm::vec3(0.0f);
}
