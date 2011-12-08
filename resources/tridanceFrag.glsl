#version 110
varying float depth;
uniform float clipDistance;

void main()
{
	if( depth < clipDistance ) discard;

	gl_FragColor.rgb	= vec3( depth );
	gl_FragColor.a		= 1.0;
}





