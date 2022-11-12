#include "simulation.h"
#include <iostream>
#include <cmath>

const size_t N = 128;

int Simulation::init() {
    mesh = Mesh();
    plane = Entity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), GLIDER);
	return 0;
}

int Simulation::update(double dt) {
    elapsed += dt;
    plane.rotation.z += sin(dt) * 0.05;
	return 0;
}
