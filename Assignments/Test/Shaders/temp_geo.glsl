#version 400 core

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

// Input vertex data, aggregated into triangles
in vec3 normal_tes[];
in vec3 position_tes[];
in vec3 eyeDirection_tes[];
in vec3 lightDirection_tes[];
in vec2 uv_tes[];

// Output data per vertex, passed to primitive assembly and rasterization
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragLightDirection;
out vec3 fragEyeDirection;
out vec2 fragUV;


// Uniform values that stay constant for the whole mesh.
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


void main() {

    for(int i = 0; i < gl_in.length(); ++i) {
        fragPosition = position_tes[i];
        fragNormal = normal_tes[i];
        fragLightDirection = lightDirection_tes[i];
        fragEyeDirection = eyeDirection_tes[i];
        fragUV = uv_tes[i];
        gl_Position = projection * view * model * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
