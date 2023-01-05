#include "simulation.h"
#include <iostream>
#include <cmath>

const size_t N = 128;

int Simulation::init() {
    mesh = Mesh();
    plane = Entity(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f)), GLIDER);
    plane.rotm = glm::rotate(plane.rotm, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    plane.velocity = glm::inverse(plane.rotm) * glm::vec4(0.0f, 0.0f, 45.0f, 0.0f);
	return 0;
}

int Simulation::update(double dt) {
    elapsed += dt;

    glm::vec4 gravity(0.0f, -9.8f, 0.0f, 0.0f);
    glm::vec4 thrust(0.0f, 0.0f, -1000.0f, 0.0f);
    // plane.velocity = glm::inverse(plane.rotm) * glm::vec4(0.0f, 0.0f, 45.0f, 0.0f);
    plane.acceleration = glm::inverse(plane.rotm) * gravity;
    plane.thrust = thrust;

    plane.update(dt);
	return 0;
}
