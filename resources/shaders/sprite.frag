#version 150 core // GLSL 1.50 Core
precision mediump float;

uniform vec4 color;
uniform sampler2D spriteTex;
uniform int colorChannels;

in vec2 fragTexCoord;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(spriteTex, fragTexCoord);

    vec4 convertedTexColor = vec4(0);
    switch (colorChannels) {
        case 1:
            convertedTexColor = vec4(vec3(texColor.r), 1);
            break;
        case 2:
            convertedTexColor = vec4(vec3(texColor.r), texColor.g);
            break;
        case 3:
            convertedTexColor = vec4(texColor.rgb, 1);
            break;
        default:
        case 4:
            convertedTexColor = texColor;
            break;
    }

    fragColor = convertedTexColor * color;
}