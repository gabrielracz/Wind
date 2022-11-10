#ifndef VIEW_H
#define VIEW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>	//vector print
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/common.hpp>

#include <cstring>


#include "camera.h"
#include "paths.h"
#include "shader.h"
#include "shapebuilder.h"


#define NUM_SHADERS 10
#define CLAMP(f) f / 255.0f

class Application;

class View {
	typedef struct {
		GLFWwindow* ptr;
		std::string title;
		int width;
		int height;
	} Window;

	typedef struct MouseType{
		bool first_captured = true;
		bool captured = true;
		double xprev, yprev;
	} Mouse;

	enum ShaderPrograms {
		DEFAULT,
		GRID,
		LIGHT,
		TEXT
	};

	enum Textures {
		CHARMAP = 0
	};

public:
	Window win;
	FreeCamera camera;
	Mouse mouse;
	ShapeBuilder shapebuilder;
	Shader shaders[NUM_SHADERS];
	Application* app = nullptr;

	unsigned int textures[3];

	//graphics stuff
	unsigned int VBO_cube;
	unsigned int VAO;
	unsigned int cube_count;

;
	View(const std::string& win_title, int win_width, int win_height);
	View();
	int Init(Application* parent);
	int Render(double dt);
	void RenderText(const std::string& text, float x, float y, float size);

private:

	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	static unsigned int InitQuad();
	static unsigned int LoadTexture(const std::string& file_path);

};
#endif
