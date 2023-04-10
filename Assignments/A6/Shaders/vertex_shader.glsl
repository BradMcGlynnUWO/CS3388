#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 cameraView;
uniform float time;

out vec3 position_worldspace;
out vec3 normal_worldspace;
out vec3 lightDirection;
out vec3 eyeDirection;
out vec2 uv;

void main()
{
    vec4 position_camera_space = vec4(aPos,1);
    vec4 normal_camera_space = view * model * vec4(aNorm,0);

    position_worldspace = position_camera_space.xyz;
    normal_worldspace = normal_camera_space.xyz;
    
    eyeDirection = cameraView - position_worldspace;

    lightDirection = lightPos - eyeDirection;
    
    uv = (position_worldspace.xz + vec2(0, 0) + (mod(time, 100.0) * 0.08)) / 1;
    gl_Position = vec4(aPos, 1.0);
}

