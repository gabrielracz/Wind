#ifndef APPLICATION_H
#define APPLICATION_H

#include "view.h"
#include "model.h"

class Application {
public:
	Application();
	int Init();
	int Run();
	void Shutdown();

	Model model;
private:
	View view;
	bool running = false;
};

#endif
