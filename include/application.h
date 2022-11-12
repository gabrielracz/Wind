#ifndef APPLICATION_H
#define APPLICATION_H

#include "view.h"
#include "simulation.h"

class Application {
public:
    Application();
    int init();
	int run();
	void shutdown();

    float fps = 0;
private:
    Simulation sim;
    View view;
	bool running = false;
};

#endif
