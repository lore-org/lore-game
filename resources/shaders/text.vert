#version 150 core // GLSL 1.50 Core
precision highp float;

uniform mat4 orthoMat;
uniform vec2 atlasSize;
uniform vec2 rectOrigin;
uniform float rotateDeg;

in vec2 vertPos;
in vec2 texCoord;

out vec2 fragTexCoord;

vec2 rotatePointAroundOrigin(vec2 point, vec2 origin, float deg) {
    // clockwise rotation
    float rads = -radians(deg);

    float cosVal = cos(rads);
    float sinVal = sin(rads);

    vec2 translatedPoint = point - origin;

    return vec2(
        (translatedPoint.x * cosVal - translatedPoint.y * sinVal) + origin.x,
        (translatedPoint.x * sinVal + translatedPoint.y * cosVal) + origin.y
    );
}

void main() {
    vec2 rotatedVertPos = rotatePointAroundOrigin(vertPos, rectOrigin, rotateDeg);
    gl_Position = orthoMat * vec4(rotatedVertPos, 0.0, 1.0);
    fragTexCoord = texCoord / atlasSize;
}