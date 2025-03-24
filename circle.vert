#version 460 core
layout(location = 0) in vec2 aPos; // vértices del círculo
layout(location = 1) in vec2 instancePos;
layout(location = 2) in vec3 instanceColor;
layout(location = 3) in float instanceRadius;

out vec3 vColor;
out vec2 vLocalCoord;

uniform vec2 u_resolution;

void main() {
    vec2 scaled = aPos * instanceRadius + instancePos;
    vec2 ndc = scaled / u_resolution * 2.0 - 1.0;
    ndc.y *= -1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    vColor = instanceColor;
    vLocalCoord = aPos;
}
