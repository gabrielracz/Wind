// Source code of vertex shader
#version 330

// Vertex buffer
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;

// Uniform (global) buffer
uniform mat4 transformation_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

// Attributes forwarded to the fragment shader
out vec2 uv_interp;

void main()
{
    // Transform vertex
	vec4 vertex_pos = vec4(vertex.xyz, 1.0);
	gl_Position = projection_matrix * view_matrix * transformation_matrix * vertex_pos;
//	gl_Position = view_matrix * vertex_pos;

    // Pass attributes to fragment shader
    //color_interp = vec4(color, 1.0);
    uv_interp = uv;
}
