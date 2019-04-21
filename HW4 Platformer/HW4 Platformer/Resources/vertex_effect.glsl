attribute vec4 position;

varying vec4 pos;

void main()
{
	pos = position;
	gl_Position = pos;
}