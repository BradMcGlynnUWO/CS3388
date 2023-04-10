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


void phongColor() {
    // Light emission properties
    vec4 LightColor = vec4(1, 1, 1, 1);

    // Material properties
    vec4 MaterialDiffuseColor = texture(tex, uv);
    vec4 MaterialAmbientColor = vec4(0.2, 0.2, 0.2, MaterialDiffuseColor.a) * MaterialDiffuseColor;
    vec4 MaterialSpecularColor = vec4(0.7, 0.7, 0.7, MaterialDiffuseColor.a);

    vec3 n = normalize( Normal_cameraspace );
    vec3 l = normalize( LightDirection_cameraspace );
    float cosTheta = clamp( dot( n,l ), 0,1 ); //ensure dot product is between 0 and 1

    vec3 E = normalize(EyeDirection_cameraspace);
    vec3 R = reflect(-l,n);
    float cosAlpha = clamp( dot( E,R ), 0,1 );

    color =
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
