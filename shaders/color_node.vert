precision mediump float;

attribute vec4 xy;
attribute vec4 in_rgba;

uniform vec4 px_offset;
uniform mat4 projection;
uniform float layer;

varying vec4 rgba;

void main() {
	vec4 px_position = vec4(xy.xy + px_offset.xy + px_offset.zw, 0, 1.0);
	vec4 normed_position = projection * px_position;
	gl_Position = vec4(normed_position.xy, layer, 1.0);
	rgba = in_rgba;
}


