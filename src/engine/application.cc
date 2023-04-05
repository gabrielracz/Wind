#include "application.h"

Application::Application() 
	: view("[]", 1080, 720)
 {}

int Application::init()
{
	view.init(this, &sim);
	view.render_loading();
	sim.init();
	return 0;
}

int Application::run()
{
	srand(time(NULL));
	float current_time = 0;
	float last_time = glfwGetTime();
	float delta_time = 0;
	float acc_delta_time = 0;
    int   fps_update_interval = 60;
	unsigned int frame_counter = 0;
	running = true;
	while(running){
		//Get frame rate
		frame_counter++;
		current_time = glfwGetTime();
		delta_time = current_time - last_time;
		acc_delta_time += delta_time;
		if(frame_counter % fps_update_interval == 0){
            fps = (float)fps_update_interval/acc_delta_time;
			acc_delta_time = 0;
		}
		last_time = current_time;

		sim.update(delta_time);
		view.render(delta_time);
	}
	return 0;
}

void Application::shutdown()
{
	running = false;
}
