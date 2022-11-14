#version 330 compatibility

uniform float uS0, uT0, uD;
in vec2 vST;

void
main()
{
	vec3 myColor = vec3( 1., 0.5, 0. );
	float xdif = vST.s-0.25;
	float ydif = vST.t-0.5;
	float dist = ((xdif**2) + (ydif**2))**0.5

	if( uS0 - uD/2. <= vST.s && vST.s <= uS0 + uD/2. && uT0 - uD/2. <= vST.t && vST.t <= uT0 + uD/2. )
	{
		myColor = vec3( 1., 1., 1. );		
	}

	if( 2. <= 5. )
	{
		myColor = vec3( 1., 0., 0. );
	}

	gl_FragColor = vec4( myColor, 1. );
}