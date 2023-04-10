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
uniform float time;
uniform sampler2D disptex;


vec3 Gerstner(vec3 worldPos, float w, float A, float phi, float Q, vec2 D, int N) {
    float wx = w * D.x;
    float wz = w * D.y;
    vec2 xzPos = worldPos.xz;
    float cosTerm = cos(w * dot(D, xzPos) + phi * time);
    float sinTerm = sin(w * dot(D, xzPos) + phi * time);
    float Qi = Q / (w * A * float(N));

    return vec3(Qi * A * D.x * cosTerm, A * sinTerm, Qi * A * D.y * cosTerm);
}


void main() {
    vec3 pos[3];

    for(int i = 0; i < gl_in.length(); ++i) {
        float displacement = clamp(texture(disptex, uv_tes[i]).r, -0.025, 0.025);
        pos[i] = position_tes[i];
        pos[i].y += displacement;

        // Add Gerstner waves
        pos[i] += Gerstner(position_tes[i], 4, 0.08, 1.1, 0.75, vec2(0.3, 0.6), 4);
        pos[i] += Gerstner(position_tes[i], 2, 0.05, 1.1, 0.75, vec2(0.2, 0.866), 4);
        pos[i] += Gerstner(position_tes[i], 0.6, 0.2, 0.4, 0.1, vec2(0.3, 0.7), 4);
        pos[i] += Gerstner(position_tes[i], 0.9, 0.15, 0.4, 0.1, vec2(0.8, 0.1), 4);
    }

    vec3 mynorm = normalize(cross(pos[1] - pos[0], pos[2] - pos[0]));

    for(int i = 0; i < gl_in.length(); ++i) {
        fragPosition = vec3(projection * view * model * vec4(pos[i], 1));
        fragNormal = mynorm;
        fragLightDirection = lightDirection_tes[i];
        fragEyeDirection = eyeDirection_tes[i];

        fragUV = uv_tes[i];

        gl_Position = projection * view * model * vec4(pos[i], 1.0);
        EmitVertex();
    }
    EndPrimitive();
}