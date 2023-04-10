#version 440 core

layout (vertices = 4) out;

in vec3 position_worldspace[];
in vec3 normal_worldspace[];
in vec3 lightDirection[];
in vec3 eyeDirection[];
in vec2 uv[];

out vec3 position_tcs[];
out vec3 normal_tcs[];
out vec3 lightDirection_tcs[];
out vec3 eyeDirection_tcs[];
out vec2 uv_tcs[];

float tessLevelInner = 16.0;
float tessLevelOuter = 16.0;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    
    gl_TessLevelInner[0] = tessLevelInner;
    gl_TessLevelInner[1] = tessLevelInner;
    gl_TessLevelOuter[0] = tessLevelOuter;
    gl_TessLevelOuter[1] = tessLevelOuter;
    gl_TessLevelOuter[2] = tessLevelOuter;
    gl_TessLevelOuter[3] = tessLevelOuter;
    
    position_tcs[gl_InvocationID] = position_worldspace[gl_InvocationID];
    normal_tcs[gl_InvocationID] = normal_worldspace[gl_InvocationID];
    lightDirection_tcs[gl_InvocationID] = lightDirection[gl_InvocationID];
    eyeDirection_tcs[gl_InvocationID] = eyeDirection[gl_InvocationID];
    uv_tcs[gl_InvocationID] = uv[gl_InvocationID];
}
