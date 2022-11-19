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
#include <unordered_set>
#include <unordered_map>


#include "freecamera.h"
#include "paths.h"
#include "shader.h"
#include "shapebuilder.h"
#include "defs.h"
#include "mesh.h"
#include "entity.h"


#define NUM_SHADERS 10
#define CLAMP(f) f / 255.0f

class Application;
class Simulation;

namespace Colors {
    const glm::vec4 None        = {0.0f, 0.0f, 0.0f, 0.0f};
    const glm::vec4 Black       = {0.0f, 0.0f, 0.0f, 1.0f};
    const glm::vec4 Grey        = {0.45f, 0.45f, 0.45f, 0.45f};
    const glm::vec4 LGrey       = {0.7f, 0.7f, 0.7f, 1.0f};
    const glm::vec4 Cream       = {0xFF/255.0f, 0xD7/255.0f, 0xAF/255.0f, 1.0f};
    const glm::vec4 White       = {1.0f, 1.0f, 1.0f, 1.0f};
    const glm::vec4 Magenta     = {1.0f, 0.0f, 1.0f, 1.0f};
    const glm::vec4 Purple      = {CLAMP(0x67), CLAMP(0x5c), CLAMP(0xff), 1.0f };
    const glm::vec4 Red         = {1.0f, 0.0f, 0.0f, 1.0f};
    const glm::vec4 Green       = {0.0f, 1.0f, 0.0f, 1.0f};
    const glm::vec4 Gasoline    = {0x87/255.0f, 0xFF/255.0f, 0xAF/255.0f, 1.0f};
    const glm::vec4 Blue        = {0.0f, 0.0f, 1.0f, 1.0f};
    const glm::vec4 Stormy      = {41/255.0f, 0x4F/255.0f, 0x4F/255.0f, 1.0f};
    const glm::vec4 Amber       = {0xFF/255.0f, 0xB0/255.0f, 0.0f, 1.0f};
    const glm::vec4 Pred        = {0xff/255.0f, 0x00/255.0f, 0x5d/255.0f, 0.0f};

}

typedef struct VertexArray {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
}VertexArray;

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
		TEXT,
        SH_LINE,
        N_SHADERS
	};

	enum Textures {
		CHARMAP = 0,
        CRATE,
        N_TEXTURES
	};

    typedef struct ControlKey {
        int key_code;
        bool hold;
        ControlKey(int key, int hld)
        : key_code(key), hold(hld){}
    }ControlKey;

public:
    Application* app;
    Simulation* sim;
	Window win;
	FreeCamera camera;
	Mouse mouse;


	ShapeBuilder shapebuilder;

    std::unordered_map<int, bool> key_controls;

	Shader shaders[N_SHADERS];
	Texture textures[N_TEXTURES];
    unsigned int text_vao;
    VertexArray line_vao;
    Mesh meshes[N_ENTITY_TYPES];

	View(const std::string& win_title, int win_width, int win_height);
	View();
	int init(Application* app, Simulation* model);
    int init_controls();
    int check_controls();
	int render(double dt);
    void render_entity(Entity& ent, const glm::vec4& color);
	void render_text(const std::string& text, float x, float y, float size, const glm::vec4& color);
    void render_line(const glm::vec3& line, const glm::vec3& color = Colors::Red, float scale = 1.0f, const glm::vec3& shift = glm::vec3(0.0f));

    static bool DRAW_WIREFRAME;

private:

	static void callback_mouse_move(GLFWwindow* window, double xpos, double ypos);
	static void callback_scroll(GLFWwindow* window, double xpos, double ypos);
	static void callback_mouse_button(GLFWwindow* window, int button, int action, int mods);
	static void callback_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void callback_resize_framebuffer(GLFWwindow* window, int width, int height);

	static unsigned int init_quad();
    static VertexArray init_line();
	static Texture load_texture(const std::string& file_path);

};
#endif
