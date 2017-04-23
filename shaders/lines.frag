#version 310 es

precision mediump float;

layout(location = 1) uniform vec3 color;
out vec4 fragmentColor;

void main() { fragmentColor = vec4(color,1.0); }
