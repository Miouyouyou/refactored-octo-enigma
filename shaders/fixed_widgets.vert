#version 310 es

precision mediump float;

layout(location = 0) in vec4 xyz;
layout(location = 1) in vec2 in_st;

layout(location = 1) uniform vec4 px_offset;
layout(location = 2) uniform mat4 px_to_norm;
out vec2 st;

void main() {
	vec4 px_position = xyz + px_offset;
	vec4 result = px_to_norm * px_position;
	gl_Position = result;
	st = in_st;
}


