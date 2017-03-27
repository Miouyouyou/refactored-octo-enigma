#version 310 es

precision highp float;

layout(location = 0) in vec4 xyz;
layout(location = 1) in vec2 in_st;

layout(location = 1) uniform vec4 px_offset;
layout(location = 2) uniform mat4 px_to_norm;
layout(location = 3) uniform float layer;
out vec2 st;

void main() {
	vec4 px_position = vec4(xyz.xy + px_offset.xy + px_offset.zw, xyz.z, 1.0);
	vec4 normed_position = px_to_norm * px_position;
	gl_Position = vec4(normed_position.xy, layer, 1.0);
	st = in_st;
}


