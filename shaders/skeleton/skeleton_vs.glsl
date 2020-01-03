#version 430

layout (location = 0) in vec3 a_position;

void main() {
    gl_PointSize = 12.0;
    gl_Position = vec4(a_position.xy, 0, 1.0);
}