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
uniform vec3 lightPos;
//uniform sampler2D heightTexture;
//uniform sampler2D normalTexture;

void main() {

    uv = uv_coord;

    // Fetch height and normal data from textures
    //float height = texture(heightTexture, uv).r;
    //vec3 normal = texture(normalTexture, uv).rgb;

    // Adjust vertex position and normal based on fetched data
    //vec3 adjustedVertexPosition = vertexPosition_modelspace + vec3(0.0, height, 0.0);
    //vec3 adjustedNormal = normalize(vertexNormal_modelspace + normal);

    gl_Position =  MVP * vec4(vertexPosition_modelspace, 1);

    vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1)).xyz;
    EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;

    vec3 LightPosition_cameraspace = (V * vec4(lightPos, 1)).xyz;
    LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

    Normal_cameraspace = (V * M * vec4(vertexNormal_modelspace, 0)).xyz;
}
