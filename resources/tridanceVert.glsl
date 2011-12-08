uniform sampler2D depthTex;
varying vec4 vVertex;
varying float depth;
uniform float clipDistance;
uniform float depthFactor;

void main()
{
	gl_TexCoord[0]		= gl_MultiTexCoord0;
	vVertex				= vec4( gl_Vertex );
	
	depth				= texture2D( depthTex, gl_TexCoord[0].st ).b;
	
	vVertex.y			+= (depth - clipDistance) * depthFactor;

	gl_Position			= gl_ModelViewProjectionMatrix * vVertex;
}
