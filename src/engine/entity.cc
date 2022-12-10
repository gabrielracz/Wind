#include "entity.h"
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/matrix.hpp>
#include <ios>
#include <iostream>
#include <iomanip>
#include <glm/gtx/string_cast.hpp>

Wing::Wing(const glm::vec3 &pos, float span, float chord, float pitch, float dihedral, float ld)
    : pos(pos), span(span), chord(chord), area(span*chord), aspect_ratio(span/chord), lift_distribution(ld), dihedral(dihedral)
{
    glm::vec3 rot(pitch, 0.0f, dihedral);
    if(glm::length(rot) > 0.0f)
        rotm = glm::rotate(glm::mat4(1.0f), glm::length(rot), rot);
}

glm::vec3 Wing::solve(const glm::vec3& air) {
    glm::vec3 force;

    glm::mat4 plane_to_wing = glm::inverse(rotm);
    glm::vec3 relwind = plane_to_wing * glm::vec4(air, 1.0f);
    glm::vec3 relwind_yz(0.0f, relwind.y, relwind.z);
    glm::vec3 chordline = plane_to_wing * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    float aoa = glm::orientedAngle( chordline, glm::normalize(relwind_yz),glm::vec3(0.0f, 1.0f, 0.0f));
    std::cout << "AoA: " << std::fixed << std::setprecision(6) << aoa;

    return force;
}

Entity::Entity(const glm::vec3 &_position, const glm::vec3 &_rotation, EntityID _id)
    : position(_position),
    rotation(_rotation),
    id(_id),
    rwing(glm::vec3(0.0f, 0.0f, 0.4f), 7, 1, 0.0, 0.00),
    lwing(glm::vec3(0.0f, 0.0f, 0.4f), -7, 1, 0.0, 0.00), //neg dihedral
	elevator(glm::vec3(0.0f, 1.25f, 4.5f), 1, 1, -0.03, 0.0f, 0.0f)
{}

void Entity::update(float dt) {
	
	//solve the wings.
	//2π⋅AR/2+sqrt(AR^2+4)
    //solve one wing
    glm::vec3 air = -velocity;
    lwing.solve(air);
    std::cout << " ##### ";
    rwing.solve(air);
    std::cout << std::endl;
    //lift curve = sin(5.5x) peeks at 16 degrees angle of attack


    //integrate accel, use
    //figure out xy and yx componenst of intertia and why they induce rotation
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
