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

    vec3 n = normalize( fragNormal );
    vec3 l = normalize( fragLightDirection );
    float cosTheta = clamp( dot( n,l ), 0,1 ); //ensure dot product is between 0 and 1

    vec3 E = normalize(fragEyeDirection);
    vec3 R = reflect(-l,n);
    float cosAlpha = clamp( dot( E,R ), 0,1 );

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
