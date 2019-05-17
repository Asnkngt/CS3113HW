uniform sampler2D diffuse;
varying vec2 texCoordVar;
uniform vec4 color;

void main() {
	gl_FragColor = texture2D(diffuse, texCoordVar);
	gl_FragColor = vec4(gl_FragColor.r*color.r,gl_FragColor.g*color.g,gl_FragColor.b*color.b,gl_FragColor.a*color.a);
	if(gl_FragColor.a == 0.0){
		discard;
	}
}