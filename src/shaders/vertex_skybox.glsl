#version 330 core

layout (location = 0) in vec3 position;

out vec3 texture_coordinates;

uniform mat4 transformation;
uniform mat4 projection;
uniform mat4 view;

void main() {
    mat4 view = mat4(mat3(view));  // fix skybox at the origin of the camera
    texture_coordinates = position;
    gl_Position = projection * view * transformation * vec4(position, 1.0f);
}
