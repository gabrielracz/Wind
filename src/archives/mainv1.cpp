#define GLEW_STATIC
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "util/stb_image.h"

#define CLAMP(int) int / 255.0f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window, Shader& shader);
std::string& load_text_file(const char* filename);
unsigned int load_shader(const char* filename, unsigned int shader_type);


int main() {
	//Version config
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//For MacOS:
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(600, 600, "First Principles", NULL, NULL);
	if (window == nullptr) {
		std::cout << "Failed to create the GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glViewport(0, 0, 600, 600);		//Perspective projection matrix
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glewInit();

	int num_vertexattributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &num_vertexattributes);
	std::cout << "[LOG] maximum number of vertex attributes: " << num_vertexattributes << std::endl;

	//Hello Triangle
	float tri_vertices[] = {
		//positions        //colors
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	float rect_vertices[] = {
		0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};

	unsigned int rect_indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	stbi_set_flip_vertically_on_load(true);

	//Texture
	int width, height, num_channels;
	unsigned char* data = stbi_load("resources/crate.jpg", &width, &height, &num_channels, 0);
	if (!data) {
		std::cout << "[ERROR] failed to laod texture" << std::endl;
		exit(-1);
	}

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	int width2, height2, num_channels2;
	unsigned char* data2 = stbi_load("resources/mossyrock.jpg", &width2, &height2, &num_channels2, 0);
	if (!data2) {
		std::cout << "[ERROR] failed to laod texture" << std::endl;
		exit(-1);
	}

	unsigned int texture2;
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	//Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data2);
	

	float tex_vertices[] = {
		// positions      // colors        // texture coords
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top right
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
		-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f // top left
	};
	
	//Manual vertex buffer object style
	unsigned int VBO, VAO, EBO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex_vertices), tex_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_indices), rect_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*) (6*sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Shader shader("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
	shader.use();
	glUniform1i(glGetUniformLocation(shader.id, "texture1"), 0);
	glUniform1i(glGetUniformLocation(shader.id, "texture2"), 1);

	while (!glfwWindowShouldClose(window)) {
		process_input(window, shader);

		glClearColor(30.0f/255, 35.0f/255, 36.0f/255, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//shader.use();

		//float time = glfwGetTime();
		//float green = sin(time) / 2.0f + 0.5f;
		//float red = sin(time*2) / 2.0f + 0.5f;
		//float blue = sin(time*1.5) / 2.0f + 0.5f;
		//int color_location = glGetUniformLocation(shader.ID, "our_color");
		//glUniform4f(color_location, red, green, blue, 1.0f);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

unsigned int load_shader(const char* filename, unsigned int shader_type) {
	std::string content = load_text_file(filename);
	const char* src = content.c_str();

	//std::cout << content << std::endl;

	unsigned int shader;
	shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &src, NULL);		//Expects an array of strings, thats why &vtx_src
	glCompileShader(shader);

	int success;
	char log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, log);
		std::cout << "[ERROR] shader compilation failed\n" << log << std::endl;
		exit(-1);
	}
	return shader;
}

std::string& load_text_file(const char* filename) {
	std::ifstream f;
	f.open(filename);
	if (f.fail()) {
		//throw(std::ios_base::failure(std::string("Error opening file ") + std::string(filename)));
		std::cout << "[ERROR] could not open file " << filename << std::endl;
		exit(-1);
	}

	std::string* content = new std::string();
	std::string line;
	while (std::getline(f, line)) {
		*content += line + '\n';
	}

	//std::cout << content << std::endl;

	f.close();
	return *content;
}

void process_input(GLFWwindow* window, Shader& shader) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		shader.load();
		std::cout << "[LOG] shaders recompiled" << std::endl;
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

