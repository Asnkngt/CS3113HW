uniform sampler2D diffuse;
uniform sampler2D light;
varying vec2 texCoordVar;

//uniform vec4 color;

void main() {
	vec4 col1 = texture2D(diffuse, texCoordVar);
	vec4 col2 = texture2D(light, texCoordVar);
	gl_FragColor = vec4(col1.r*col2.r,col1.g*col2.g,col1.b*col2.b,col1.a*col2.a)+col2*0.1;
	//gl_FragColor = vec4(gl_FragColor.r*color.r,gl_FragColor.g*color.g,gl_FragColor.b*color.b,gl_FragColor.a*color.a);
}