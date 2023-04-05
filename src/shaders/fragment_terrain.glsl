#version 330 core
in vec3 frag_pos_og;
in vec2 frag_uv;
in vec3 frag_normal;

uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec4 base_color;
//out vec4 FragColor;

uniform sampler2D texture1;


void main(){
	float pixel = 0.5f;
	vec3 frag_pos = vec3(frag_pos_og.x - mod(frag_pos_og.x, pixel), 
			frag_pos_og.y - mod(frag_pos_og.y, pixel),
			frag_pos_og.z - mod(frag_pos_og.z, pixel));
	
    float ambient_strength = 0.4f;
    vec3 ambient = ambient_strength * light_color;

    vec3 normal = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);

    float diffuse_strength = max(dot(normal, light_dir), 0.0f);
    vec3 diffuse = diffuse_strength * light_color;

    float slope = 1 - normal.y;
    float grassSlopeThreshold = 0.25;
    float grassBlendAmount = 0.6;
    float grassBlendHeight = grassSlopeThreshold * (1-grassBlendAmount);
    float grassWeight = 1- clamp((slope-grassBlendHeight)/(grassSlopeThreshold-grassBlendHeight), 0, 1);
    vec4 grassColor = vec4(0xb8/255.0, 0xbb/255.0, 0x26/255.0, 1.0);
    vec4 rockColor = vec4(0x50/255.0, 0x49/255.0, 0x45/255.0, 1.0);
    vec4 tex_color = grassColor * grassWeight + rockColor * (1-grassWeight);

    //		vec4 tex_color = texture(texture1, frag_uv);
    vec4 tex_color = base_color;
    vec3 result = (ambient + diffuse) * tex_color.rgb;

    gl_FragColor = vec4(result, 1.0f);

}
