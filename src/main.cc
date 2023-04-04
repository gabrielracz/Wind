#define USE_MATH_DEFINES_
#define _USE_MATH_DEFINES
#define GLM_FORCE_RADIANS
#include <cmath>
#include "application.h"


int main(int argc, char** argv) {
    Application app;
    app.init();
	app.run();
	return 0;
}

