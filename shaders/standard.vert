#version 310 es

precision highp float;

layout(location = 0) in vec2 xyz;

layout(location = 0) uniform vec2 offset;

void main() {
	gl_Position = vec4(xyz + offset, 0.5, 1.0);
}
