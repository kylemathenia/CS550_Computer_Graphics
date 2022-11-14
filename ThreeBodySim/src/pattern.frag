#version 330 compatibility

uniform float uS0, uT0, uD;
in vec2 vST;

void
main()
{
	vec3 myColor = vec3( 1., 0.5, 0. );
	float xdif = vST.s-0.25;
	float xdif_sq = pow(xdif,2.0);
	float ydif = vST.t-0.5;
	float ydif_sq = pow(ydif,2.0);
	float sum = xdif_sq + ydif_sq;
	float dist = pow(sum,0.5);

	if( dist <= 0.1 )
	{
		myColor = vec3( 1., 0., 0. );
	}

	gl_FragColor = vec4( myColor, 1. );
}