#version 330 compatibility

uniform float uS0, uT0, uD;
in vec2 vST;

void
main()
{
	vec3 myColor = vec3( 1., 0.5, 0. );
	if( uS0 - uD/2. <= vST.s && vST.s <= uS0 + uD/2. && uT0 - uD/2. <= vST.t && vST.t <= uT0 + uD/2. )
	{
		myColor = vec3( 1., 0., 0. );		
	}
	gl_FragColor = vec4( myColor, 1. );
}