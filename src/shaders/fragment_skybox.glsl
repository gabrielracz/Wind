#version 330 core

in vec3 texture_coordinates;
uniform samplerCube skybox;

const mat4 dither_kernel = 
    mat4(
        0, 8, 2, 10,
        12, 4, 14, 6,
        3, 11, 1, 9,
        15, 7, 13, 5
    ) * 1.0f/16.0f;

vec4 dither(vec4 color) {

    float intensity = (color.r + color.g + color.b)/3.0f;
    int pixel_x = int(gl_FragCoord.x);
    int pixel_y = int(gl_FragCoord.y);
    float threshhold = dither_kernel[pixel_x % 4][pixel_y % 4] + 0.5f;
    vec4 result;
    if(intensity > threshhold){
        //result = vec4(0x69/255.0f, 0xC0/255.0f, 0xDF/255.0f, 1.0f);
        result = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        result = vec4(0.0f);
    }
    return result;
}

void main() {
        gl_FragColor = dither(texture(skybox, texture_coordinates));
}
