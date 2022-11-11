#ifndef MODEL_H
#define MODEL_H
#include "mesh.h"

class Simulation {
public:
    Mesh mesh;
	Simulation() = default;
	int init();
	int update(double dt);
};

#endif
