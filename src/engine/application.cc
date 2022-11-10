#include "application.h"

Application::Application() 
	: view("[]", 640, 420)
 {}

int Application::Init() {
	view.Init(this);
	model.Init();
	return 0;
}

int Application::Run() {
	srand(time(NULL));
	float current_time = 0;
	float last_time = glfwGetTime();
	float delta_time = 0;
	float acc_delta_time = 0;
	unsigned int frame_counter = 0;
	running = true;
	while(running){
		//Get frame rate
		frame_counter++;
		current_time = glfwGetTime();
		delta_time = current_time - last_time;
		acc_delta_time += delta_time;
		if(frame_counter % 60 == 0){
			std::cout << "FPS: " << std::to_string(60/acc_delta_time) << std::endl;
			acc_delta_time = 0;
		}
		last_time = current_time;

		model.Step(delta_time);
		view.Render(delta_time);
	}
	return 0;
}

void Application::Shutdown() {
	running = false;
	return;
}
