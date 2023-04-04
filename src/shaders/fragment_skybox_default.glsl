#version 330 core

in vec3 texture_coordinates;
uniform samplerCube skybox;
void main() {
        gl_FragColor = texture(skybox, texture_coordinates);
}
