#version 330 compatibility

uniform float	uA;		// "Time", from Animate( )
out vec2  	vST;		// texture coords

float uTime = uA;
const float PI = 	3.14159265;
const float AMP = 	0.2;		// amplitude
const float W = 	2.;		// frequency

void
main()
{ 
	vST = gl_MultiTexCoord0.st;
	vec3 vert = gl_Vertex.xyz;
	vert.x = vert.x*(AMP*sin(uTime)+1);
	vert.y = vert.y*(AMP*sin(2*uTime)+1);
	if( vert.z >= 0) {vert.z = vert.z*(AMP*sin(3*uTime)+0.5);}
	gl_Position = gl_ModelViewProjectionMatrix * vec4( vert, 1. );
}