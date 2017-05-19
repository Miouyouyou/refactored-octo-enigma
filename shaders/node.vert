precision mediump float;

attribute vec4 xyz;
attribute vec2 in_st;

uniform vec4 px_offset;
uniform mat4 px_to_norm;
uniform float layer;

varying vec2 st;

void main() {
	vec4 px_position = vec4(xyz.xy + px_offset.xy + px_offset.zw, xyz.z, 1.0);
	vec4 normed_position = px_to_norm * px_position;
	gl_Position = vec4(normed_position.xy, layer, 1.0);
	st = in_st;
}
