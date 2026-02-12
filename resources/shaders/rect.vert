#version 150 core
precision highp float;

uniform vec2 viewportSize;
uniform vec2 rectOrigin;
uniform float rotateDeg;

in vec2 vertPos;

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

    gl_Position = vec4(
        ((rotatedVertPos.x / viewportSize.x) * 2.0) - 1.0,
        ((rotatedVertPos.y / viewportSize.y) * 2.0) - 1.0,
        0.0, 1.0
    );
}