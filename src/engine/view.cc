#include "view.h"
#include "freecamera.h"
#include "application.h"
#include "mesh.h"
#include "shapes.h"
#include "simulation.h"
#include <GLFW/glfw3.h>
#include <linux/limits.h>
#include <paths.h>

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool View::DRAW_WIREFRAME = false;
bool View::DRAW_DEBUG = false;

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
	//  glfwSwapInterval(0);

	//  Shaders
	shaders[S_DEFAULT]   = Shader(SHADER_DIRECTORY"/vertex_default.glsl",  SHADER_DIRECTORY"/fragment_default.glsl");
	shaders[S_DITHER]   = Shader(SHADER_DIRECTORY"/vertex_default.glsl",  SHADER_DIRECTORY"/fragment_dither.glsl");
	shaders[S_DITHER2]   = Shader(SHADER_DIRECTORY"/vertex_default.glsl",  SHADER_DIRECTORY"/fragment_dither2.glsl");
	shaders[S_TEXT]      = Shader(SHADER_DIRECTORY"/vertex_text.glsl",     SHADER_DIRECTORY"/fragment_text.glsl");
    shaders[S_LINE]      = Shader(SHADER_DIRECTORY"/vertex_line.glsl",     SHADER_DIRECTORY"/fragment_line.glsl");
    shaders[S_SKYBOX]    = Shader(SHADER_DIRECTORY"/vertex_skybox.glsl",   SHADER_DIRECTORY"/fragment_skybox.glsl");


    textures[T_CRATE]   = load_texture(RESOURCES_DIRECTORY"/crate_large.jpg");
	textures[T_CHARMAP] = load_texture(RESOURCES_DIRECTORY"/fixedsys_alpha.png");

    //right, left, top, bottom, front, back
#define SKYBOX_NAME "sunnysky"
    std::vector<std::string> skybox_faces({
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_ft.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_bk.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_up.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_dn.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_rt.jpg",
          RESOURCES_DIRECTORY"/" SKYBOX_NAME"/" SKYBOX_NAME "_lf.jpg"
    });
    textures[T_SKYBOX] = load_cubemap(skybox_faces);
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

	glm::vec3 camera_position(0.0f, 4.0f, 17.0f);
	glm::vec3 camera_front(0.0f, 0.0f, -1.0f);
	glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
	camera = Camera(camera_position, camera_front, camera_up);
    camera.look_sensitivity = 0.0025;

	//Mouse
	mouse.xprev = (float) win.width / 2;
	mouse.yprev = (float) win.height / 2;

	init_controls();

    light_pos = glm::vec3(4.0f, 170.0f, 0.0f);
    camera.target = &sim->plane;
	return 0;
}

int View::init_controls() {
    key_controls.insert({GLFW_KEY_I, false});
	key_controls.insert({GLFW_KEY_K, false});
	key_controls.insert({GLFW_KEY_L, false});
	key_controls.insert({GLFW_KEY_J, false});
	key_controls.insert({GLFW_KEY_0, false});
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
    //pitch
	if(key_controls[GLFW_KEY_W]) {
        sim->plane.elevator.change_pitch(Pitch::Up);
	} else if(key_controls[GLFW_KEY_S]) {
        sim->plane.elevator.change_pitch(Pitch::Down);
	} else {
        sim->plane.elevator.change_pitch(Pitch::Neutral);
    }

    //roll
	if(key_controls[GLFW_KEY_D]) {
        sim->plane.rwing.change_pitch(Pitch::Up);
        sim->plane.lwing.change_pitch(Pitch::Down);
	} else if(key_controls[GLFW_KEY_A]) {
        sim->plane.rwing.change_pitch(Pitch::Down);
        sim->plane.lwing.change_pitch(Pitch::Up);
	} else {
        sim->plane.rwing.change_pitch(Pitch::Neutral);
        sim->plane.lwing.change_pitch(Pitch::Neutral);
    }

	if(key_controls[GLFW_KEY_Q]) {
        sim->plane.rudder.change_pitch(Pitch::Up);
	} else if(key_controls[GLFW_KEY_E]) {
        sim->plane.rudder.change_pitch(Pitch::Down);
	} else {
        sim->plane.rudder.change_pitch(Pitch::Neutral);
    }

    if(key_controls[GLFW_KEY_I]) {
        camera.Rotate(MoveDirection::DOWN);
    }
    if(key_controls[GLFW_KEY_K]) {
        camera.Rotate(MoveDirection::UP);
    }
    if(key_controls[GLFW_KEY_J]) {
        camera.Rotate(MoveDirection::RIGHT);
    }
    if(key_controls[GLFW_KEY_L]) {
        camera.Rotate(MoveDirection::LEFT);
    }
    if(key_controls[GLFW_KEY_0]) {
        camera.Rotate(MoveDirection::RESET);
    }

	if(key_controls[GLFW_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(win.ptr, true);
	}

    sim->plane.throttle = !key_controls[GLFW_KEY_SPACE];
	// camera.move_forward = key_controls[GLFW_KEY_W];
    // camera.move_back = key_controls[GLFW_KEY_S];
    // camera.move_left = key_controls[GLFW_KEY_A];
    // camera.move_right = key_controls[GLFW_KEY_D];
	if(key_controls[GLFW_KEY_RIGHT_BRACKET]) {
		View::DRAW_DEBUG = !View::DRAW_DEBUG;
		key_controls[GLFW_KEY_RIGHT_BRACKET] = false;
	}
	return 0;
}

int View::render(double dt)
{
	glfwPollEvents();
    framedelta = dt;
	check_controls();

	if(glfwWindowShouldClose(win.ptr))
		app->shutdown();

    glm::vec4 clr = Colors::Black;
	glClearColor(clr.r, clr.g, clr.b, clr.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // camera.position = glm::vec3(0.0f, 4.0f, -15.0f) + sim->plane.position;
    camera.Update();

    render_skybox();
    render_terrain(Colors::Grey);
    render_aircraft(sim->plane, Colors::White);

    // render hud overtop


    render_hud();

	glfwSwapBuffers(win.ptr);
	return 0;
}

void View::render_aircraft(Aircraft& aircraft, const glm::vec4& color)
{
    glm::mat4 translation = glm::translate(aircraft.position);
    glm::mat4& rotation  = aircraft.rotm;
    glm::mat4 transform = translation * rotation;
    const glm::mat4 reverse_z = glm::rotate(glm::pi<float>(), glm::vec3(0.f, 1.f, 0.f));

    if(View::DRAW_DEBUG) {
        render_wing_forces(aircraft.lwing, transform*reverse_z, rotation);
        render_wing_forces(aircraft.rwing, transform*reverse_z, rotation);
        render_wing_forces(aircraft.elevator, transform, rotation);
        render_wing_forces(aircraft.rudder, transform, rotation);
        render_line(rotation * glm::vec4(aircraft.velocity, 1.0f), Colors::Cyan, 1.0f, aircraft.position);
        // if(aircraft.throttle)
        //     render_line(rotation * glm::vec4(aircraft.thrust, 1.0f), Colors::Green, 1.0f, aircraft.position);
    }

    Shader& shd = shaders[S_DITHER];
    shd.use();
    shd.SetUniform4f(color, "base_color");
    shd.SetUniform3f(Colors::White, "light_color");
    shd.SetUniform3f(light_pos, "light_pos");
    shd.SetUniform4m(transform, "model");
    shd.SetUniform4m(camera.projection, "projection");
    shd.SetUniform4m(camera.view, "view");

    Mesh& mesh = meshes[aircraft.id];
    mesh.Draw(shd);
}

void View::render_terrain(const glm::vec4& color) {
    glm::mat4 transform(1.0f);
    Shader& shd = shaders[S_DITHER2];
    shd.use();
    shd.SetUniform4f(color, "base_color");
    shd.SetUniform3f(Colors::White, "light_color");
    shd.SetUniform3f(light_pos, "light_pos");
    shd.SetUniform4m(transform, "model");
    shd.SetUniform4m(camera.projection, "projection");
    shd.SetUniform4m(camera.view, "view");

    const glm::mat4 dither_kernel2 =
        glm::mat2(
            0, 2,
            3, 1
        ) * 1.0f/4.0f;

    const glm::mat4 dither_kernel4 = 
        glm::mat4(
            0, 8, 2, 10,
            12, 4, 14, 6,
            3, 11, 1, 9,
            15, 7, 13, 5
        ) * 1.0f/16.0f;

    shd.SetUniform4m(dither_kernel4, "kernel");
    shd.SetUniform1i(win.width, "window_width");
    shd.SetUniform1i(win.height, "window_height");

    sim->terrain.Draw(shd);
}

void View::render_wing_forces(Wing wing, glm::mat4 transform, glm::mat4 rotation) {
    // rotate the wing forces by 180 to be relative to +z direction (-z by default)
    glm::vec3 rwing_pos = transform * glm::vec4(wing.pos + wing.center_of_pressure, 1.0f);
    glm::vec3 lift = rotation * wing.rotm * glm::vec4(wing.lift, 1.0f);
    render_line(lift, Colors::Green, 0.001f, rwing_pos);

    glm::vec3 drag = rotation * wing.rotm * glm::vec4(wing.drag, 1.0f);
    render_line(drag, Colors::Pred, 0.001f, rwing_pos);

    glm::vec3 net = rotation * glm::vec4(wing.net_force, 1.0f);
    render_line(net, Colors::Magenta, 0.001f, rwing_pos);
}

void View::render_skybox() {
    glDepthMask(false);
    Shader& shd = shaders[S_SKYBOX];
    shd.use();
    shd.SetUniform4m(glm::scale(glm::vec3(300.0f)), "transformation");
    shd.SetUniform4m(camera.view, "view");
    shd.SetUniform4m(camera.projection, "projection");
    glBindVertexArray(skybox_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures[T_SKYBOX].id);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(true);
}

void View::render_hud() {
    float text_size = 15;
    const auto row_spacing = [&] (float row) {
        float start = row <= 0.0f ? -1.0f : 1.0f;
        return start-(text_size/((float)(win.height)/2.0f) * (2.0f * (row-1.0f)));
    };

    char fps[32];
    std::sprintf(fps, "%-7.4f %3s", app->fps, "fps");
    render_text(fps, -1, 1, text_size, Colors::GGreen);

    char frame_time[32];
    std::sprintf(frame_time, "%-7.4f %3s", framedelta, "ms");
    render_text(frame_time, -1, row_spacing(2), text_size, Colors::GGreen);

    char airspeed[32];
    std::sprintf(airspeed, "%-7.2f %4s", glm::length(sim->plane.velocity) * 3.6, "km/h");
    render_text(airspeed, 1, 1, text_size, Colors::GGreen, TextPosition::TOPRIGHT);

    char altitude[32];
    std::sprintf(altitude, "%-7.2f %4s", sim->plane.position.y, "m");
    render_text(altitude, 1, row_spacing(2), text_size, Colors::GGreen, TextPosition::TOPRIGHT);

    char gforce[32];
    std::sprintf(gforce, "%-7.2f %4s", glm::length(sim->plane.velocity)*glm::length(sim->plane.rot_velocity)/9.8, "G");
    render_text(gforce, 1, row_spacing(3), text_size, Colors::GGreen, TextPosition::TOPRIGHT);

    char aileron_angle[64];
    char elevator_angle[64];
    char rudder_angle[64];
    char spacer[64];
    std::sprintf(aileron_angle ,"% 3.2f          % 3.2f ", sim->plane.lwing.pitch * RAD_TO_DEG, sim->plane.rwing.pitch * RAD_TO_DEG);
    std::sprintf(elevator_angle,"    % 3.2f   % 3.2f    ", sim->plane.elevator.pitch * RAD_TO_DEG, sim->plane.elevator.pitch * RAD_TO_DEG);
    std::sprintf(rudder_angle  ,"        % 3.2f        ", sim->plane.rudder.pitch * RAD_TO_DEG);

    text_size = 13.125;
    render_text(aileron_angle , 1, row_spacing(-1), text_size, Colors::GGreen, TextPosition::BOTTOMRIGHT);
    render_text(elevator_angle, 1, row_spacing(0), text_size, Colors::GGreen, TextPosition::BOTTOMRIGHT);
    render_text(rudder_angle  , 1,              -1, text_size, Colors::GGreen, TextPosition::BOTTOMRIGHT);

}

void View::render_text(const std::string& text, float x, float y, float size, const glm::vec4& color, TextPosition text_position)
{
    Shader& shd = shaders[S_TEXT];
    shd.use();
    glBindVertexArray(text_vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, textures[T_CHARMAP].id);

    int len = text.size();
    shd.SetUniform1i(len, "text_len");

    //chars are 8x15 pixels
    float width = len*8/15.0f*(size);
	float sx = width;
	float sy = size;

    float asp = 1.0f/((float)win.width/(float)win.height);
    
    float ndc_width_distance = (width/((float)(win.width)/2));
    float ndc_height_distance = (size/((float)win.height/2));

    // change the anchor point of the text quad
    glm::mat4 translation;
    switch(text_position) {
        case TOPLEFT:
            translation = glm::translate(glm::vec3(x + ndc_width_distance, y - ndc_height_distance, 0.0f));
            break;
        case BOTTOMLEFT:
            translation = glm::translate(glm::vec3(x + ndc_width_distance, y + ndc_height_distance, 0.0f));
            break;
        case TOPRIGHT:
            translation = glm::translate(glm::vec3(x - ndc_width_distance, y - ndc_height_distance, 0.0f));
            break;
        case BOTTOMRIGHT:
            translation = glm::translate(glm::vec3(x - ndc_width_distance, y + ndc_height_distance, 0.0f));
            break;
        case CENTER:
            translation = glm::translate(glm::vec3(x, y, 0.0f));
            break;
    }

    glm::mat4 transformation_matrix = translation * glm::scale(glm::vec3(sx, sy, 1.0f));

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
	// const float sensitivity = 0.001f;
	// xoffset *= sensitivity;
	// yoffset *= sensitivity;

    if(yoffset > 0.0f) {
        v->camera.Rotate(MoveDirection::DOWN);
    }
    else if(yoffset <= 0.0f) {
        v->camera.Rotate(MoveDirection::UP);
    }
    if(xoffset > 0.0f) {
        v->camera.Rotate(MoveDirection::RIGHT);
    }
    else if(xoffset < 0.0f) {
        v->camera.Rotate(MoveDirection::LEFT);
    }

    // v->camera.StepYaw(xoffset);
	// v->camera.StepPitch(-yoffset);
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

    stbi_set_flip_vertically_on_load(0);
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

    Texture t = {tex_id, "skybox"};
	return t;
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
