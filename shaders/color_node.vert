#version 310 es

precision mediump float;

layout(location = 0) in vec4 xy;
layout(location = 1) in vec4 in_rgba;

layout(location = 1) uniform vec4 px_offset;
layout(location = 2) uniform mat4 projection;
layout(location = 3) uniform float layer;
out vec4 rgba;

void main() {
	vec4 px_position = vec4(xy.xy + px_offset.xy + px_offset.zw, 0, 1.0);
	vec4 normed_position = projection * px_position;
	gl_Position = vec4(normed_position.xy, layer, 1.0);
	rgba = in_rgba;
}


