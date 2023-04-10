#version 400

// Interpolated values from the vertex shaders
in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragLightDirection;
in vec3 fragEyeDirection;
in vec2 fragUV;

// Output data
out vec4 color_out;

uniform sampler2D waterTexture;

void phongColor() {
    // Light emission properties
    vec4 LightColor = vec4(1, 1, 1, 1);

    // Material properties
    vec4 MaterialDiffuseColor = texture(waterTexture, fragUV);
    vec4 MaterialAmbientColor = vec4(0.2, 0.2, 0.2, MaterialDiffuseColor.a) * MaterialDiffuseColor;
    vec4 MaterialSpecularColor = vec4(0.7, 0.7, 0.7, MaterialDiffuseColor.a);

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(fragLightDirection);
    vec3 V = normalize(fragEyeDirection);


    float cosTheta = clamp(dot(N, L), 0, 1);
    vec3 R = reflect(-L, N);
    float cosAlpha = clamp(dot(V, R), 0, 1);


    color_out =
        // Ambient: simulates indirect lighting
        MaterialAmbientColor +
        // Diffuse: "color" of the object
        MaterialDiffuseColor * LightColor * cosTheta +
        // Specular: reflective highlight, like a mirror
        MaterialSpecularColor * LightColor * pow(cosAlpha, 8);
}

void main() {
    phongColor();
}



