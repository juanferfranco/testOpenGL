#version 460 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    vec2 pos = aPos / vec2(400.0, 300.0) - 1.0;
    pos.y *= -1.0;
    gl_Position = vec4(pos, 0.0, 1.0);
    vColor = aColor;
}
