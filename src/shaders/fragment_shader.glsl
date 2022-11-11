#version 330 core

in vec3 frag_pos;
in vec2 frag_uv;
in vec3 frag_normal;

uniform vec3 light_color;
uniform vec3 light_pos;

//out vec4 FragColor;

uniform sampler2D texture1;


void main(){
	float ambient_strength = 0.4f;
	vec3 ambient = ambient_strength * light_color;

	vec3 normal = normalize(frag_normal);
	vec3 light_dir = normalize(light_pos - frag_pos);

	float diffuse_strength = max(dot(normal, light_dir), 0.0f);
	vec3 diffuse = diffuse_strength * light_color;

	vec4 tex_color = texture(texture1, frag_uv);
//	vec4 tex_color = vec4(0.5f, 1.0f, 0.5, 1.0f);
	vec3 result = (ambient + diffuse) * tex_color.rgb;

	gl_FragColor = vec4(result, 1.0f);

}
