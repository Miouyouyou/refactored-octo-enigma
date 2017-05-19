precision mediump float;

attribute vec4 xyz;
attribute vec4 in_rgba;

uniform vec4 px_offset;
uniform mat4 px_to_norm;

varying vec4 rgba;

void main() {
	vec4 px_position = xyz + px_offset;
	vec4 result = px_to_norm * px_position;
	gl_Position = result;
	rgba = in_rgba;
}


