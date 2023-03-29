#version 330 core
in vec3 frag_pos;
in vec2 frag_uv;
in vec3 frag_normal;

uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec4 base_color;
uniform mat4 kernel;
uniform int window_width;
uniform int window_height;
//out vec4 FragColor;

uniform sampler2D texture1;

const mat4 dither_kernel = 
    mat4(
        0, 8, 2, 10,
        12, 4, 14, 6,
        3, 11, 1, 9,
        15, 7, 13, 5
    ) * 1.0f/16.0f;

vec4 dither(vec4 color) {

    float intensity = (color.r + color.g + color.b)/3.0f;
    int pixel_x = int(gl_FragCoord.x)/int(2);
    int pixel_y = int(gl_FragCoord.y)/int(2);
    float threshhold = dither_kernel[pixel_x % 4][pixel_y % 4] + 0.3f;
    vec4 result;
    if(intensity > threshhold){
        // result = color;
        result = vec4(1.0f);
    } else {
        result = vec4(0.0f);
    }
    return result;
}

void main(){
	float pixel = 0.5f;
	
    float ambient_strength = 0.4f;
    vec3 ambient = ambient_strength * light_color;

    vec3 normal = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);

    float diffuse_strength = max(dot(normal, light_dir), 0.0f);
    vec3 diffuse = diffuse_strength * light_color;

    //		vec4 tex_color = texture(texture1, frag_uv);
    vec4 tex_color = base_color;
    vec3 result = (ambient + diffuse) * tex_color.rgb;

    gl_FragColor = dither(vec4(result, 1.0f));
    // gl_FragColor = vec4(result, 1.0f);

}
