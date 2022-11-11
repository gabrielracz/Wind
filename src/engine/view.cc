#include "view.h"
#include "camera.h"
#include "application.h"
#include "simulation.h"
#include <paths.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

View::View(const std::string& win_title, int win_width, int win_height)
{
	win.title = win_title;
	win.height = win_height;
	win.width = win_width;
}

View::View()
{
	float aspect_ratio = 16.0f / 9.0f;
	int win_height = 960;
	int window_width = round(win_height * aspect_ratio);
	View("__ Plume", win_height, window_width);
}

int View::init(Application* parent, Simulation* model)
{
    app = parent;
    sim = model;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//For MacOS:
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	win.ptr = glfwCreateWindow(win.width, win.height, win.title.c_str(), nullptr, nullptr);
	if (win.ptr == nullptr) {
		std::cout << "Failed to create the GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(win.ptr);

    glfwSetWindowUserPointer(win.ptr, this);
	glfwSetInputMode(win.ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(win.ptr,       callback_mouse_move);
    glfwSetMouseButtonCallback(win.ptr,     callback_mouse_button);
    glfwSetScrollCallback(win.ptr,          callback_scroll);
	glfwSetKeyCallback(win.ptr,             callback_keyboard);
    glfwSetFramebufferSizeCallback(win.ptr, callback_resize_framebuffer);

    glewExperimental = GL_TRUE;
    glewInit();
    glViewport(0, 0, win.width, win.height);		//Perspective projection matrix

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glfwSwapInterval(0);

	//Shaders
	shaders[DEFAULT] = Shader(SHADER_DIRECTORY"/vertex_shader.glsl",
					  SHADER_DIRECTORY"/fragment_shader.glsl");
//
//	shaders[GRID] = Shader(SHADER_DIRECTORY"/vertex_shader.glsl",
//					  SHADER_DIRECTORY"/fragment_shader.glsl");
//
//	shaders[LIGHT] = Shader(SHADER_DIRECTORY"/vertex_shader.glsl",
//					  SHADER_DIRECTORY"/fragment_shader_light.glsl");

	shaders[TEXT] = Shader(SHADER_DIRECTORY"/text_vertex.glsl",
					  SHADER_DIRECTORY"/text_fragment.glsl");

    textures[CRATE]   = load_texture(RESOURCES_DIRECTORY"/crate_large.jpg");
	textures[CHARMAP] = load_texture(RESOURCES_DIRECTORY"/fixedsys_alpha.png");

	VAO = init_quad();
    Layout layout = {
            {FLOAT3, "position"},
            {FLOAT2, "uv"},
            {FLOAT3, "normal"}
    };
    meshes[CUBE] = Mesh(ShapeBuilder::cube_vertices_tn, sizeof(ShapeBuilder::cube_vertices_tn)/sizeof(float),
                        {}, 0,
                        &(textures[CRATE]), 1,
                        layout);

	glm::vec3 camera_position(0.0f, 2.5f, 5.0f);
	glm::vec3 camera_front(0.0f, 0.0f, -1.0f);
	glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
	camera = FreeCamera(camera_position, camera_front, camera_up);

	//Mouse
	mouse.xprev = (float) win.width / 2;
	mouse.yprev = (float) win.height / 2;

	return 0;
}

int View::render(double dt)
{
	if(glfwWindowShouldClose(win.ptr))
		app->shutdown();

    glm::vec4 clr = Colors::White;
	glClearColor(clr.r, clr.g, clr.b, clr.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.Update();
    char fps_str[32];
    std::sprintf(fps_str, "%.4f ms", dt);
	render_text(fps_str, 0.0, 0.0, 1.0f, Colors::Black);
    render_mesh(meshes[CUBE]);

	glfwSwapBuffers(win.ptr);
	glfwPollEvents();
	return 0;
}

void View::render_mesh(Mesh& mesh)
{
    Shader& shd = shaders[DEFAULT];
    shd.use();
    shd.SetUniform3f(Colors::White, "light_color");
    shd.SetUniform3f(glm::vec3(0.0f, 4.0f, 0.0f), "light_pos");
    shd.SetUniform4m(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)), "model");
    shd.SetUniform4m(camera.projection, "projection");
    shd.SetUniform4m(camera.view, "view");
    mesh.Draw(shd);

}

void View::render_text(const std::string& text, float x, float y, float size, const glm::vec4& color)
{
    glUseProgram(shaders[TEXT].id);
    glBindVertexArray(VAO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[CHARMAP].id);

	int len = text.size();
	int loc_text_len = glGetUniformLocation(shaders[TEXT].id, "text_len");
	glUniform1i(loc_text_len, len);

    float width = len*8/15.0f*(size);
	float sx = width;
	float sy = size;

    glm::mat4 transformation_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, 1.0f));

	float asp = 1.0f/((float)win.width/(float)win.height);
    glm::mat4 view_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(asp, 1.0f, 1.0f));
//    glm::mat4 view_matrix(1.0f);

	int loc_transform = glGetUniformLocation(shaders[TEXT].id, "transformation_matrix");
	glUniformMatrix4fv(loc_transform, 1, GL_FALSE, glm::value_ptr(transformation_matrix));

    shaders[TEXT].SetUniform4m(camera.view, "view_matrix");
    shaders[TEXT].SetUniform4m(camera.projection, "projection_matrix");
    shaders[TEXT].SetUniform4f(color, "text_color");

	// Set the text data
	assert(512 > len);
	int data[512] = {0};
	for (int i = 0; i < len; i++){
		data[i] = text[i];
	}

	int loc_content = glGetUniformLocation(shaders[TEXT].id, "text_content");
	glUniform1iv(loc_content, len, data);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
    glDisable(GL_BLEND);
}

//  STATICS:
static int cnt = 0;
void View::callback_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	View* v = (View*) glfwGetWindowUserPointer(window);
	Mouse& mouse = v->mouse;
	if (mouse.first_captured) {
		mouse.xprev = xpos;
		mouse.yprev = ypos;
		mouse.first_captured = false;
	}
	float xoffset = xpos - mouse.xprev;
	float yoffset = ypos - mouse.yprev;

	mouse.xprev = xpos;
	mouse.yprev = ypos;
	const float sensitivity = 0.001f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;


    v->camera.StepYaw(xoffset);
	v->camera.StepPitch(-yoffset);
}

void View::callback_scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	View* v = (View*) glfwGetWindowUserPointer(window);
	v->camera.StepFov(-yoffset);
}

void View::callback_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	View* v = (View*) glfwGetWindowUserPointer(window);
	Mouse& mouse = v->mouse;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (mouse.captured) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(window, nullptr);
			mouse.captured = false;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPosCallback(window, callback_mouse_move);
			mouse.captured = true;
			mouse.first_captured = true;
		}
	}
}

void View::callback_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	View* v = (View*) glfwGetWindowUserPointer(window);
	FreeCamera* cam = &v->camera;


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
		if (v->shaders[DEFAULT].load()) {
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

void View::callback_resize_framebuffer(GLFWwindow* window, int width, int height)
{
    View* v = (View*) glfwGetWindowUserPointer(window);
    v->win.width = width;
    v->win.height = height;
    glViewport(0, 0, width, height);
}


unsigned int View::init_quad()
{
	float vertices[] = {
		 0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f,  0.0f, 1.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0); 
	return VAO;
}

Texture View::load_texture(const std::string& file_path)
{
	stbi_set_flip_vertically_on_load(1);

	//Texture
	int width, height, n_channels;
	unsigned char* data = stbi_load(file_path.c_str(), &width, &height, &n_channels, 0);
	if (!data) {
		printf("ERROR: failed to load image %s\n", file_path.c_str());
		return Texture{};
	}

    GLenum format;
    switch(n_channels) {
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            format = GL_RGB;
            break;
    }

	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	//Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
    Texture t = {tex, "NAMELESS"};
	return t;
}
