#ifndef MODEL_H
#define MODEL_H
#include "mesh.h"
#include "entity.h"

class Simulation {
public:
    Mesh mesh;
    Entity plane;
    unsigned int elapsed = 0;
	Simulation() = default;
	int init();
	int update(double dt);
};

#endif
