#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out vec3 frag_pos;
out vec2 frag_uv;
out vec3 frag_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	frag_pos = vec3(model * vec4(position, 1.0f));
	frag_normal = mat3(transpose(inverse(model))) * normal;
	frag_uv = uv;
	gl_Position = projection * view * model * vec4(frag_pos, 1.0f);
}
