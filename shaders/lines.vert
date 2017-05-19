precision mediump float;

attribute vec2 xyz;

uniform vec2 offset;

void main() {
	gl_Position = vec4(xyz + offset, 0.5, 1.0);
}
