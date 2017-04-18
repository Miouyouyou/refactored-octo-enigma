#version 310 es

precision mediump float;

out vec4 fragmentColor;

in vec4 rgba;

void main() { fragmentColor = rgba; }
