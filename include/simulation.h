#ifndef MODEL_H
#define MODEL_H
#include "mesh.h"
#include "aircraft.h"

class Simulation {
public:
    Mesh terrain;
    Aircraft plane;
    unsigned int elapsed = 0;
	Simulation() = default;
	int init();
    void gen_terrain();
	int update(double dt);
};

#endif
