#include "view.h"
#include "freecamera.h"
#include "application.h"
#include "mesh.h"
#include "shapes.h"
#include "simulation.h"
#include <GLFW/glfw3.h>
#include <paths.h>

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool View::DRAW_WIREFRAME = false;

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
	//  For MacOS:
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
    glViewport(0, 0, win.width, win.height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
//    glfwSwapInterval(0);

	//  Shaders
	shaders[S_DEFAULT]   = Shader(SHADER_DIRECTORY"/vertex_default.glsl",  SHADER_DIRECTORY"/fragment_default.glsl");
	shaders[S_TEXT]      = Shader(SHADER_DIRECTORY"/vertex_text.glsl",     SHADER_DIRECTORY"/fragment_text.glsl");
    shaders[S_LINE]      = Shader(SHADER_DIRECTORY"/vertex_line.glsl",     SHADER_DIRECTORY"/fragment_line.glsl");
    shaders[S_SKYBOX]    = Shader(SHADER_DIRECTORY"/vertex_skybox.glsl",   SHADER_DIRECTORY"/fragment_skybox.glsl");


    textures[T_CRATE]   = load_texture(RESOURCES_DIRECTORY"/crate_large.jpg");
	textures[T_CHARMAP] = load_texture(RESOURCES_DIRECTORY"/fixedsys_alpha.png");


    //right, left, top, bottom, front, back
#define SKYBOX_NAME "wrath"
    std::vector<std::string> skybox_faces({
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_ft.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_bk.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_up.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_dn.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_rt.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_lf.jpg"
    });
    stbi_set_flip_vertically_on_load(0);
    textures[T_SKYBOX]   = load_cubemap(skybox_faces);
    skybox_vao = init_cube();

	

    //  Raw OpenGL buffers
    text_vao = init_quad();
    line_vao = init_line();

    //  3D meshes
    Layout layout = {
            {FLOAT3, "position"},
            {FLOAT2, "uv"},
            {FLOAT3, "normal"}
    };
    meshes[CUBE] = Mesh(Shapes::cube_vertices_tn, sizeof(Shapes::cube_vertices_tn)/sizeof(float),
                        {}, 0,
                        &(textures[T_CRATE]), 1,
                        layout);
    meshes[GLIDER] = Mesh(RESOURCES_DIRECTORY"/glider2.obj");

	glm::vec3 camera_position(0.0f, 1.0f, -1.0f);
	glm::vec3 camera_front(0.0f, 0.0f, -1.0f);
	glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
	camera = FreeCamera(camera_position, camera_front, camera_up);

	//Mouse
	mouse.xprev = (float) win.width / 2;
	mouse.yprev = (float) win.height / 2;

	init_controls();

	return 0;
}

int View::init_controls() {
    key_controls.insert({GLFW_KEY_I, false});
	key_controls.insert({GLFW_KEY_K, false});
	key_controls.insert({GLFW_KEY_L, false});
	key_controls.insert({GLFW_KEY_J, false});
	key_controls.insert({GLFW_KEY_U, false});
	key_controls.insert({GLFW_KEY_O, false});

	key_controls.insert({GLFW_KEY_W, false});
	key_controls.insert({GLFW_KEY_A, false});
	key_controls.insert({GLFW_KEY_S, false});
	key_controls.insert({GLFW_KEY_D, false});
    key_controls.insert({GLFW_KEY_E, false});
    key_controls.insert({GLFW_KEY_Q, false});


	key_controls.insert({GLFW_KEY_RIGHT_BRACKET, false});
	key_controls.insert({GLFW_KEY_ESCAPE, false});
	return 0;
}

int View::check_controls() {
	if(key_controls[GLFW_KEY_I]) {
		sim->plane.rot_acceleration.x = -10;
	}
	if(key_controls[GLFW_KEY_K]) {
		sim->plane.rot_acceleration.x = 10;
	}
	if(key_controls[GLFW_KEY_L]) {
		sim->plane.rot_acceleration.z = 10;
	}
	if(key_controls[GLFW_KEY_J]) {
		sim->plane.rot_acceleration.z = -10;
	}
	if(key_controls[GLFW_KEY_U]) {
		sim->plane.rot_acceleration.y = 10;
	}
	if(key_controls[GLFW_KEY_O]) {
		sim->plane.rot_acceleration.y = -10;
	}

	if(key_controls[GLFW_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(win.ptr, true);
	}

	camera.move_forward = key_controls[GLFW_KEY_W];
    camera.move_back = key_controls[GLFW_KEY_S];
    camera.move_left = key_controls[GLFW_KEY_A];
    camera.move_right = key_controls[GLFW_KEY_D];
	if(key_controls[GLFW_KEY_RIGHT_BRACKET]) {
		View::DRAW_WIREFRAME = !View::DRAW_WIREFRAME;
		key_controls[GLFW_KEY_RIGHT_BRACKET] = false;
	}
	return 0;
}

int View::render(double dt)
{
	glfwPollEvents();
	check_controls();

	if(glfwWindowShouldClose(win.ptr))
		app->shutdown();

    glm::vec4 clr = Colors::Black;
	glClearColor(clr.r, clr.g, clr.b, clr.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.Update();

    render_skybox();

    render_entity(sim->plane, Colors::LGrey);

    // render hud overtop
    char frame_time[32];
    std::sprintf(frame_time, "%.4f ms", dt);
    render_text(frame_time, -0.79, 0.9, 15, Colors::Green);

    char fps[32];
    std::sprintf(fps, "%.2f  fps", app->fps);
    render_text(fps, -0.775, 0.8, 15, Colors::Green);

	glfwSwapBuffers(win.ptr);
	return 0;
}

void View::render_entity(Entity& ent, const glm::vec4& color)
{
    glm::mat4 transform(1.0f);
    glm::mat4 translation(1.0f);
    glm::mat4 rotation(1.0f);
    //pitch, yaw, roll to rotation matrix
    translation = glm::translate(glm::mat4(1.0f), ent.position);
    rotation  = ent.rotm;
    transform = translation * rotation;

    // glm::mat4 rwing_transform = glm::translate(glm::mat4(1.0f), ent.rwing.pos) * ent.rwing.rotm;
    // glm::vec3 rwing_norm = transform * rwing_transform * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    // render_line(rwing_norm, Colors::Amber, 90.0f, rwing_pos);

    render_wing_forces(ent.lwing, transform, rotation);
    render_wing_forces(ent.rwing, transform, rotation);
    render_wing_forces(ent.elevator, transform, rotation);


    Shader& shd = shaders[S_DEFAULT];
    shd.use();
    shd.SetUniform4f(color, "base_color");
    shd.SetUniform3f(Colors::White, "light_color");
    shd.SetUniform3f(glm::vec3(4.0f, 10.0f, 0.0f), "light_pos");
    shd.SetUniform4m(transform, "model");
    shd.SetUniform4m(camera.projection, "projection");
    shd.SetUniform4m(camera.view, "view");

    Mesh mesh = meshes[ent.id];
    mesh.Draw(shd);

}

void View::render_wing_forces(Wing wing, glm::mat4 transform, glm::mat4 rotation) {

    glm::vec3 rwing_pos = transform * glm::vec4(wing.pos + wing.center_of_pressure, 1.0f);
    glm::vec3 lift = rotation * wing.rotm * glm::vec4(wing.lift, 1.0f);
    render_line(lift, Colors::Green, 1.5f, rwing_pos + glm::vec3(0.0f, 0.3f, 0.0f));

    glm::vec3 drag = rotation * wing.rotm * glm::vec4(wing.drag, 1.0f);
    render_line(drag, Colors::Pred, 1.5f, rwing_pos + glm::vec3(0.0f, 0.3f, 0.0f));

    glm::vec3 net = rotation * wing.rotm * glm::vec4(wing.net_force, 1.0f);
    render_line(net, Colors::Magenta, 1.5f, rwing_pos + glm::vec3(0.0f, 0.3f, 0.0f));
}

void View::render_skybox() {
    glDepthMask(false);
    Shader& shd = shaders[S_SKYBOX];
    shd.use();
    shd.SetUniform4m(glm::scale(glm::mat4(1.0f), glm::vec3(300.0f)), "transformation");
    shd.SetUniform4m(camera.view, "view");
    shd.SetUniform4m(camera.projection, "projection");
    glBindVertexArray(skybox_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures[T_SKYBOX].id);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(true);
}

void View::render_text(const std::string& text, float x, float y, float size, const glm::vec4& color)
{
    Shader& shd = shaders[S_TEXT];
    shd.use();
    glBindVertexArray(text_vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, textures[T_CHARMAP].id);

    int len = text.size();
    shd.SetUniform1i(len, "text_len");

    float width = len*8/15.0f*(size);
	float sx = width;
	float sy = size;

    float asp = 1.0f/((float)win.width/(float)win.height);
    glm::mat4 transformation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    transformation_matrix = glm::scale(transformation_matrix, glm::vec3(sx, sy, 1.0f));

    glm::mat4 view_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(asp, 1.0f, 1.0f));

    shd.SetUniform4m(transformation_matrix, "transformation_matrix");

    float w = (float)win.width/2.0f;
    float h = (float)win.height/2.0f;
    shd.SetUniform4m(glm::ortho<float>(-w, w, -h, h, -1.0f, 1.0f), "projection_matrix");
    shd.SetUniform4f(color, "text_color");

	// Set the text data
	assert(512 > len);
	int data[512] = {0};
	for (int i = 0; i < len; i++){
		data[i] = text[i];
	}

    shd.SetUniform1iv(data, len, "text_content");

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void View::render_line(const glm::vec3 &line, const glm::vec3 &color, float scale, const glm::vec3& shift) {
    glBindVertexArray(line_vao.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, line_vao.VBO);

    float line_verts[] = {
            0.0f, 0.0f, 0.0f,
            line.x, line.y, line.z
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, 6*sizeof(float), line_verts);

    Shader shader = shaders[S_LINE];
    shader.use();

    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, shift);
    transform = glm::scale(transform, glm::vec3(scale));

    shader.SetUniform4f(glm::vec4(color, 1.0f), "line_color");
    shader.SetUniform4m(transform, "model");
    shader.SetUniform4m(camera.view, "view");
    shader.SetUniform4m(camera.projection, "projection");

    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//  STATICS:
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

	if(action == GLFW_PRESS) {
		v->key_controls[key] = true;
	}else if(action == GLFW_RELEASE) {
		v->key_controls[key] = false;
	}

}

void View::callback_resize_framebuffer(GLFWwindow* window, int width, int height)
{
    View* v = (View*) glfwGetWindowUserPointer(window);
    v->win.width = width;
    v->win.height = height;
    v->camera.SetAspectRatio((float)width/(float)height);
    glViewport(0, 0, width, height);
}


unsigned int View::init_quad()
{
	float vertices[] = {
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f
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

VertexArray View::init_line() {
    const float line_verts[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    unsigned int VBO, VAO;
    VertexArray arr;
    glGenBuffers(1, &arr.VBO);
    glGenVertexArrays(1, &arr.VAO);
    glBindVertexArray(arr.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, arr.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_verts), line_verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return arr;
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

Texture View::load_cubemap(std::vector<std::string>& faces) {
	unsigned int tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

	int w, h, n_channels;
	for(int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &n_channels, 0);
		if(data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
			stbi_image_free(data);
		}else {
			std::cout << "[ERROR] Cube map " << faces[i] << " failed to load" << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	return {tex_id, "skybox"};
}

unsigned int View::init_cube() {
	unsigned int VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::cube_vertices), Shapes::cube_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return VAO;
};
