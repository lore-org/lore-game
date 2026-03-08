#version 150 core // GLSL 1.50 Core
precision highp float;

uniform vec4 textColor;
uniform sampler2D glyphAtlas;

in vec2 fragTexCoord;

out vec4 fragColor;

// SDF
// void main() {
//     float distFromOutline = texture(glyphAtlas, fragTexCoord).r - 0.5;
//     float distChangePerFragment = length(vec2(
//         dFdx(distFromOutline), dFdy(distFromOutline)
//     ));

//     float alpha = 1.0 - smoothstep(
//         -distChangePerFragment, distChangePerFragment,
//         distFromOutline
//     );

//     fragColor = vec4(textColor.rgb, textColor.a * alpha);
// }


void main() {
    float texAlpha = texture(glyphAtlas, fragTexCoord).r;
    fragColor = vec4(textColor.rgb, textColor.a * texAlpha);
}