#version 400

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec2 uv_coord;

// Output data ; will be interpolated for each fragment.
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec2 uv;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 P;
uniform vec3 lightPos;

// New uniforms for normal and height textures
uniform sampler2D normalTexture;
uniform sampler2D heightTexture;

void main() {
    uv = uv_coord;

    // Sample normal and height data from the textures
    vec3 normalData = texture(normalTexture, uv_coord).xyz;
    float heightData = texture(heightTexture, uv_coord).x;

    // Modify the vertex position and normal using the new data
    vec3 adjustedVertexPosition = vertexPosition_modelspace;
    vec3 adjustedNormal = normalize(vertexNormal_modelspace);

    // Replace the original vertex position and normal with the adjusted ones
    gl_Position =  P * V * M * vec4(adjustedVertexPosition, 1);

    vec3 vertexPosition_cameraspace = (V * M * vec4(adjustedVertexPosition, 1)).xyz;
    EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;

    vec3 LightPosition_cameraspace = (V * vec4(lightPos, 1)).xyz;
    LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

    Normal_cameraspace = (V * M * vec4(adjustedNormal, 0)).xyz;
}
