#version 430 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 blendColor;
uniform float blendFactor;
uniform int applyBlend; 
uniform int isPlane; 

void main() {
    if (isPlane == 1) {
        FragColor = vec4(blendColor, 1.0); // plane
    } else {
        vec4 texColor = texture(texture1, TexCoord); // Default texture
        if (applyBlend == 1) {
            vec4 texColor1 = texture(texture1, TexCoord);
            vec4 texColor2 = texture(texture2, TexCoord);
            vec4 colorBlend = mix(texColor1, texColor2, blendFactor); // two textures
            texColor = mix(colorBlend, vec4(blendColor, 1.0), blendFactor); //color
        }
        FragColor = texColor;
    }
}
