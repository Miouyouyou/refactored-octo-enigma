#version 310 es

precision mediump float;

layout(location = 0) uniform sampler2D sampler;

out vec4 fragmentColor;

in vec2 st;

void main() {
	fragmentColor = texture(sampler, st);
}
