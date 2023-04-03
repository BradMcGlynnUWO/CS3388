#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform float time;

out vec3 position_worldspace;
out vec3 normal_worldspace;
out vec3 lightDirection;
out vec3 eyeDirection;
out vec2 uv;

void main()
{
    position_worldspace = (vec4(aPos,1)).xyz;
    normal_worldspace = ( vec4(aNorm,0)).xyz;
    
    eyeDirection = vec3(0,0,0) - position_worldspace;

    vec3 LightPosition_cameraspace = ( view * vec4(lightPos,1)).xyz;
    lightDirection = LightPosition_cameraspace + eyeDirection;
    uv = (position_worldspace.xz + vec2(0, 0) + (time * 0.08)) / 1;
    gl_Position = vec4(aPos, 1.0);
}
