#version 460 core
in vec3 vColor;
in vec2 vLocalCoord;
out vec4 FragColor;

void main() {
    float dist = length(vLocalCoord);
    if (dist > 1.0) {
        discard;
    }
    float edgeWidth = 0.12; // 12% del radio (ajustable)

    if (dist > 1.0 - edgeWidth) {
        FragColor = vec4(vColor, 1.0); // borde color
    } 
}
