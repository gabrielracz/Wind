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

    glm::vec4 gravity(0.0f, -9.8f, 0.0f, 0.0f);
    plane.acceleration = glm::inverse(plane.rotm) * gravity;
    plane.update(dt);
	return 0;
}
