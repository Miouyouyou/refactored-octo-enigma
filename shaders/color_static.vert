precision mediump float;

attribute vec4 xyz;
attribute vec4 in_rgba;

uniform mat4 projection;
uniform float layer;

varying vec4 rgba;

void main() {
	vec4 normed_position = projection * xyz;
	gl_Position = vec4(normed_position.xy, layer, 1.0);
	rgba = in_rgba;
}
