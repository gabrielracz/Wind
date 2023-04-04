#include "aircraft.h"
#include "simulation.h"
#include <GLFW/glfw3.h>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/matrix.hpp>
#include <ios>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>

const float AIR_DENSITY = 1.29f;

Wing::Wing(const glm::vec3 &pos, float span, float chord, float pitch, float dihedral, float ld)
    : pos(pos), span(span), chord(chord), area(abs((int)span)*chord), lift_distribution(ld), resting_pitch(pitch), dihedral(dihedral)
{
    pitch = resting_pitch;
    update_rotation();
    //  lift distribution = point along span where lift is applied (averaged).
    //  lift is usually distributed at one quarter chord length from the leading edge.
    center_of_pressure = rotm * glm::vec4(span*lift_distribution, 0.0f, chord*0.25, 1.0f);
}

void Wing::update_rotation() {
    glm::vec3 rot(pitch, 0.0f, -dihedral);
    if(glm::length(rot) > 0.0f)
        rotm = glm::rotate(glm::mat4(1.0f), glm::length(rot), rot);
}

float Wing::coefficient_lift(float aoa) {
    float max_cl = 1.2;
    if(aoa <= stall_angle && aoa >= -stall_angle) {
        stalled = false;
        return (max_cl / stall_angle) * aoa; //linear lift
    } else {
        stalled = true;
        return sin(2*aoa);
    }
}

float Wing::coefficient_drag(float aoa) {
    return 1 - cos(2*aoa);
}

void Wing::solve_aoa(const glm::vec3& forward_air, const glm::vec3& rvelocity) {
    glm::vec3 air = forward_air;// - glm::cross(rvelocity, pos + center_of_pressure);
    // std::cout << glm::to_string(glm::cross(rvelocity, pos + center_of_pressure)) << glm::to_string(rvelocity) << std::endl;
    glm::mat4 plane_to_wing = glm::inverse(rotm);
    glm::vec3 relwind = plane_to_wing * glm::vec4(air, 1.0f);
    glm::vec3 relwind_yz = glm::vec3(0.0f, relwind.y, relwind.z);
    flow_velocity = glm::length(relwind_yz);
    rel = relwind;
    glm::vec3 wing_chord = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    angle_of_attack = atan2(glm::dot(glm::cross(wing_chord, relwind_yz), glm::vec3(1.0f, 0.0f, 0.0f)),
                                     glm::dot(wing_chord, relwind_yz));
}

glm::vec3 Wing::solve_lift() {
    float cl = coefficient_lift(angle_of_attack);
    float lift = cl * (AIR_DENSITY * flow_velocity*flow_velocity)/2 * area;
    glm::vec3 lift_vector = glm::normalize(glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), rel)) * lift;
    return lift_vector;
}

glm::vec3 Wing::solve_drag() {
    float cd = coefficient_drag(angle_of_attack);
    float drag = cd * (AIR_DENSITY * flow_velocity*flow_velocity)/2 * area;
    glm::vec3 drag_vector = glm::normalize(rel) * drag;
    return drag_vector;
}

glm::vec3 Wing::solve(const glm::vec3& air, const glm::vec3& rvelocity) {
    solve_aoa(air, rvelocity);
    lift = solve_lift();
    drag = solve_drag();
    net_force = rotm * glm::vec4(lift + drag, 1.0f);
    return rotm * glm::vec4(net_force, 1.0f);
    // return net_force;
}

void Wing::change_pitch(Pitch p) {
    switch(p) {
        case Pitch::Up:
            pitch = resting_pitch + 0.07f;
            break;
        case Pitch::Down:
            pitch = resting_pitch - 0.07f;
            break;
        case Pitch::Neutral:
            pitch = resting_pitch;
            break;
        default:
            break;
    };
    update_rotation();
}



Aircraft::Aircraft(const glm::vec3 &_position, const glm::vec3 &_rotation, EntityID _id)
    : position(_position),
    rotation(_rotation),
    id(_id),
    rwing(glm::vec3(0.0f, 0.0f, 0.4f), 7, 1, 0.0, 0.05),
    lwing(glm::vec3(0.0f, 0.0f, 0.4f), -7, 1, 0.0, -0.05), //neg dihedral
	elevator(glm::vec3(0.0f, 0.0f, 4.5f), 3, 1.0f, 0.00025f, 0.0f, 0.0f),
	rudder(glm::vec3(0.0f, 0.0f, 4.5f), -1.5f, 1.0f, 0.0f, M_PI_2)
{

}

void Aircraft::update(float dt) {
    if(dt == 0.0f) {return;}
    elapsed += dt;

    rot_acceleration = glm::vec3(0.0f);
    acceleration =  glm::vec3(0.0f);

    glm::vec3 air = -velocity; // add wind here
    lwing.solve(air, rot_velocity);
    rwing.solve(air, rot_velocity);
    elevator.solve(air, rot_velocity);
    rudder.solve(air, rot_velocity);

    glm::vec3 translational_force(0.0f);
    translational_force += rwing.net_force;
    translational_force += lwing.net_force;
    translational_force += elevator.net_force;
    translational_force += rudder.net_force;
    if(throttle)
        translational_force += thrust;

    if(velocity != glm::vec3(0.0f)){
        glm::vec3 vdir = glm::normalize(velocity);
        float vmag = glm::length(velocity);
        float drag_constant = 0.2f;
        float fuselage_length = 8.0f;
        float cross_section = fuselage_length - fuselage_length*glm::dot(vdir, glm::vec3(0.0f, 0.0f, -1.0f)) + 1.0f;
        glm::vec3 fuselage_drag = -vdir * vmag*vmag * drag_constant * cross_section;
        translational_force += fuselage_drag;
    }

    acceleration = translational_force / mass;
    acceleration += glm::vec3(glm::inverse(rotm) * Simulation::gravity);

    velocity += acceleration * dt;
    glm::vec3 world_velocity = rotm * glm::vec4(velocity, 1.0f);
    position += glm::vec3(world_velocity.x, world_velocity.y, world_velocity.z) * dt / 4.0f;

    glm::vec3 torque;
    torque += glm::cross(rwing.pos + rwing.center_of_pressure, rwing.net_force);
    torque += glm::cross(lwing.pos + lwing.center_of_pressure, lwing.net_force);
    torque += glm::cross(elevator.pos, elevator.net_force);
    torque += glm::cross(rudder.pos, rudder.net_force);

    // if(rot_velocity != glm::vec3(0.0f)) {
    //     glm::vec3 rot_air = glm::cross(rot_velocity, rwing.pos + rwing.center_of_pressure);
    //     float rmag = glm::length(rot_air);
    //     glm::vec3 rot_air_resistance = glm::normalize(rot_air) * rmag*rmag * 0.5f;
    //     torque += glm::cross(rwing.pos + rwing.center_of_pressure, rot_air_resistance);
    //     torque += glm::cross(lwing.pos + lwing.center_of_pressure, rot_air_resistance);
    // }

    rot_acceleration = glm::inverse(inertia*inertia) * torque;

    //integrate velocity
    rot_velocity += glm::vec3(rot_acceleration.x, rot_acceleration.y, -rot_acceleration.z) * dt;
    float rmag = glm::length(rot_velocity);
    rot_velocity *= 1.0f - dt*1.1; //some damping


    float theta = glm::length(rot_velocity) * dt;
    if(theta > 0.0f) {
        glm::mat4 frame_rotation = glm::rotate(glm::mat4(1.0f), theta, rot_velocity * dt);
        rotm = rotm * frame_rotation;
    }

}
