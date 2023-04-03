#version 400

// Interpolated values from the vertex shaders
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in vec2 uv;

// Output data
out vec4 color;

uniform vec4 modelcolor;
float alpha = 64;
uniform sampler2D tex;


void main() {
    color = texture(tex, uv);
}

