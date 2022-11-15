#include <cstring>
#define GLEW_STATIC
#include <iostream>
#include <memory.h>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>	//vector print
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/common.hpp>
#include "shader.h"
#include "freecamera.h"
#include "random.h"
#include "mesh.h"
#include "shapebuilder.h"
#define STB_IMAGE_IMPLEMENTATION
#include "util/stb_image.h"
#include <vector>
#include "path_config.h"

//#include "perlin.h"
#include "../src/perlin/perlin.c"

#include <unistd.h>

#include "shapebuilder.h"

#define CLM(float) float / 255.0f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
std::string& load_text_file(const char* filename);
unsigned int load_shader(const char* filename, unsigned int shader_type);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


glm::vec3 camera_position(0.0f, 2.5f, 5.0f);
glm::vec3 camera_front(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
FreeCamera camera(camera_position, camera_front, camera_up);

Shader shader(SHADER_DIRECTORY "/vertex_default.glsl", SHADER_DIRECTORY "/fragment_default.glsl");


float aspect_ratio = 16.0f / 9.0f;
int window_height = 960;
int window_width = round(window_height * aspect_ratio);

bool first_mouse_capture = true;
bool mouse_captured = true;
float xprev = (float)window_width / 2;
float yprev = (float)window_height / 2;

int grid_width = 0;

int main() {
	//Version config
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//For MacOS:
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	std::string title = "__graphics";
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, title.c_str(), NULL, NULL);
	if (window == nullptr) {
		std::cout << "Failed to create the GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glViewport(0, 0, window_width, window_height);		//Perspective projection matrix
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glewInit();

	int num_vertexattributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &num_vertexattributes);
	std::cout << "[LOG] maximum number of vertex attributes: " << num_vertexattributes << std::endl;

	RandGenerator rng;
	rng.setrange(-10, 10);
	std::vector<glm::vec3> cube_positions;

	ShapeBuilder shp;
	shp.GenerateSphere(10, 2, &verts, &inds, &num_verts, &num_inds);

	Texture t = {texture, "cool"};
	Layout layout = {
		{FLOAT3, "position"},
		{FLOAT2, "uv"},
		{FLOAT3, "normal"}
	};
	Mesh cube_mesh(ShapeBuilder::cube_vertices_tn, sizeof(ShapeBuilder::cube_vertices_tn) / sizeof(float),
				{}, 0, 
				&t, 1,
				layout);

	float* sphere_vertices;
	unsigned int* sphere_indices;
	unsigned int num_sphere_verts, num_sphere_inds;
	shp.GenerateSphere(30, 1, &sphere_vertices, &sphere_indices, &num_sphere_verts, &num_sphere_inds);
	
	Layout light_layout({
		{FLOAT3, "position"},
		{FLOAT2, "uv"}
	});

	Mesh light_mesh(sphere_vertices, num_sphere_verts,
					sphere_indices, num_sphere_inds,
					{}, 0,
					light_layout);

	delete sphere_vertices;
	delete sphere_indices;


	shader.load();
	shader.use();

	Shader light_shader(SHADER_DIRECTORY"/vertex_default.glsl", SHADER_DIRECTORY"/fragment_light.glsl");
	light_shader.load();
	light_shader.use();

	glm::vec3 light_pos(2.2f, 12.0f, 2.3f);
	glm::vec4 light_color(CLM(254.0f), CLM(253.0f), CLM(165.0f), 1.0f);

	int light_color_location = glGetUniformLocation(light_shader.id, "light_color");
	glUniform4f(light_color_location, light_color.r, light_color.g, light_color.b, light_color.a);

	int light_pos_location = glGetUniformLocation(light_shader.id, "light_pos");
	glUniform3f(light_pos_location, light_pos.x, light_pos.y, light_pos.z);

	//The transition to the third plane
	glm::mat4 model(1.0f);
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	//glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, 2560, 1440, 60);

	glfwSetWindowUserPointer(window, &camera);

	glEnable(GL_DEPTH_TEST);
	//glfwSwapInterval(0);

	unsigned int frame_count = 0;
	float prev_time = glfwGetTime();
	float current_time = 0;
	float frame_elapsed_time = 0;
	float fps;
	float delta_time = 0;
	float total_elapsed = 0;

	while (!glfwWindowShouldClose(window)) {
		//FPS
		current_time = glfwGetTime();
		delta_time = current_time - prev_time;
		frame_elapsed_time += delta_time;
		total_elapsed += delta_time;
		prev_time = current_time;
		frame_count++;
		if (frame_count == 30) {
			fps = frame_count / frame_elapsed_time;
			frame_elapsed_time = 0;
			frame_count = 0;
			std::string new_title = title + " --- fps: " + std::to_string(fps);
			glfwSetWindowTitle(window, new_title.c_str());
		}
		//4E4E4E
		//82C5D9
		glClearColor(CLM(0x82), CLM(0xC5), CLM(0xD9), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Transforms
		shader.use();
		model = glm::mat4(1.0f);

		//light_pos = camera.position;

		shader.SetUniform3f(light_color, "light_color");
		shader.SetUniform3f(light_pos, "light_pos");
		shader.SetUniform1f(glfwGetTime(), "time");
		shader.SetUniform4m(model, "model");
		shader.SetUniform4m(camera.projection, "projection");
		camera.Update();
		shader.SetUniform4m(camera.view, "view");

		cube_mesh.Draw(shader);

		grid_mesh.Draw(shader);


		model = glm::mat4(1.0f);
		model = glm::translate(model, light_pos);
		model = glm::scale(model, glm::vec3(0.2f));
		light_shader.use();
		light_shader.SetUniform4m(model, "model");
		light_shader.SetUniform4m(camera.projection, "projection");
		light_shader.SetUniform4m(camera.view, "view");

		light_mesh.Draw(light_shader);


		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (first_mouse_capture) {
		xprev = xpos;
		yprev = ypos;
		first_mouse_capture = false;
	}
	float xoffset = xpos - xprev;
	float yoffset = ypos - yprev;

	xprev = xpos;
	yprev = ypos;
	const float sensitivity = 0.001f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;



	FreeCamera* cam = (FreeCamera*) glfwGetWindowUserPointer(window);
	cam->StepYaw(xoffset);
	cam->StepPitch(-yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	FreeCamera* cam = (FreeCamera*) glfwGetWindowUserPointer(window);
	cam->StepFov(-yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (mouse_captured) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(window, nullptr);
			mouse_captured = false;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPosCallback(window, mouse_callback);
			mouse_captured = true;
			first_mouse_capture = true;
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	FreeCamera* cam = (FreeCamera*) glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) {
			cam->move_forward = true;
		} else if (action == GLFW_RELEASE) {
			cam->move_forward = false;
		}
	}
	else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS){
			cam->move_back = true;
		} else if (action == GLFW_RELEASE) {
			cam->move_back = false;
		}
	}
	else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			cam->move_left = true;
		} else if (action == GLFW_RELEASE) {
			cam->move_left = false;
		}
	}
	else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			cam->move_right = true;
		} else if (action == GLFW_RELEASE) {
			cam->move_right = false;
		}
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		if (shader.load()) {
			std::cout << "[LOG] shader loaded" << std::endl;
		}
	}
	else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		std::cout << "POS: " << glm::to_string(cam->position) << std::endl;
		std::cout << "DIR: " << glm::to_string(cam->direction) << std::endl;
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

