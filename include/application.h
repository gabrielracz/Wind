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

private:
    Simulation sim;
    View view;
	bool running = false;
};

#endif
