#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

//layout(std430, binding = 3) buffer intensities
//{
//	float data_SSBO[];
//};

out vec3 frag_pos;
out vec3 frag_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	frag_pos = vec3(model * vec4(position, 1.0f));
	frag_normal = mat3(transpose(inverse(model))) * normal;
	texture_coordinate = texcoord;
	gl_Position = projection * view * vec4(frag_pos, 1.0f);
}
