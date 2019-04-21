
uniform sampler2D diffuse;
varying vec2 texCoordVar;
uniform vec4 color;

void main() {
	float gray=texture2D(diffuse, texCoordVar).r;
	gl_FragColor= color * gray;
}