uniform vec4 color;
uniform float brightness;
uniform float offset;

varying vec4 pos;

void main() {
    vec4 c = color;
	float mag = pos.x * pos.x + pos.y * pos.y;
	mag = mag * -0.5 + 1.0;
	mag = pow(mag, 10);
	c.a = 1.75 - mag - brightness + offset;
	gl_FragColor = c;
}
