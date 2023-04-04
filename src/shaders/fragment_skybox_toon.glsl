#version 330 core

in vec3 texture_coordinates;
uniform samplerCube skybox;

vec4 quantizeGrayscale(vec3 color, int numSteps) {
    // Convert color to grayscale using the formula Y = 0.2126*R + 0.7152*G + 0.0722*B
    float gray = dot(color, vec3(0.2126, 0.7152, 0.0722));
    // Quantize the grayscale value to the nearest multiple of (1.0 / numSteps)
    float quantized = round(gray * numSteps) / numSteps;

    // Convert the quantized grayscale value back to RGB
    return vec4(quantized, quantized, quantized, 1.0f);
}

void main() {
        gl_FragColor = quantizeGrayscale(vec3(texture(skybox, texture_coordinates)), 11);
}
