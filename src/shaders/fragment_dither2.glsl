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

vec4 quantizeGrayscale(vec3 color, int numSteps) {
    // Convert color to grayscale using the formula Y = 0.2126*R + 0.7152*G + 0.0722*B
    float gray = dot(color, vec3(0.2126, 0.7152, 0.0722));
    // Quantize the grayscale value to the nearest multiple of (1.0 / numSteps)
    float quantized = round(gray * numSteps) / numSteps  - 0.3;

    // Convert the quantized grayscale value back to RGB
    return vec4(quantized, quantized, quantized, 1.0f);
}

void main(){
	float pixel = 0.5f;
	
    float ambient_strength = 0.45f;
    vec3 ambient = ambient_strength * light_color;

    vec3 normal = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);

    float diffuse_strength = max(dot(normal, light_dir), 0.0f);
    vec3 diffuse = diffuse_strength * light_color;

    //		vec4 tex_color = texture(texture1, frag_uv);
    vec4 tex_color = base_color;
    vec3 result = (ambient + diffuse) * tex_color.rgb;

    // vec4 quantized = quantizeGrayscale(result, 17);
    gl_FragColor = dither(vec4(result, 1.0f));
    // gl_FragColor = quantized;

}
