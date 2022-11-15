#include "simulation.h"
#include <iostream>
#include <cmath>

const size_t N = 128;

int Simulation::init() {
    mesh = Mesh();
    plane = Entity(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f)), GLIDER);
	return 0;
}

int Simulation::update(double dt) {
    elapsed += dt;

//    float p = plane.rotation.x;
//    float y = plane.rotation.y;
//    float r = plane.rotation.z;
//
//    // angular velocities about x, y, z aero nautical game: (z, x, y)
//    float P = 0.00f;
//    float Q = 0.000f;
//    float R = 0.01f;
//
//    float roll_v = P + Q * sin(r) * tan(p) + R * cos(r) * tan(p);
//    float pitch_v = Q * cos(r) - R * sin(r);
//    float yaw_v = Q * sin(r) * 1.0f/cos(p) + R * cos(r)*1.0f/cos(p);


    plane.rotation.z = 0.01;
    plane.rotation.x = 0.02;
    plane.update(dt);
//    plane.rotation.z += sin(dt) * 0.2;
	return 0;
}
