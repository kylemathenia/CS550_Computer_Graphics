#version 330 compatibility

uniform float uS0, uT0, uD, uB;
in vec2 vST;

float uTime = uB;
const float AMP = 	0.8;		// amplitude
const float W = 	2.;		// frequency

void
main()
{	
	vec3 myColor = vec3( 1., 0.5, 0. );

	float xdif = vST.s-0.16666;
	float xdif_sq = pow(xdif,2.0);
	float ydif = vST.t-0.5;
	float ydif_sq = pow(ydif,2.0);
	float sum = xdif_sq + ydif_sq;
	float dist = pow(sum,0.5);
	if( dist <= 0.1 * (AMP*sin(uTime)+0.2)) {myColor = vec3( 1., 0., 0. );}

	xdif = vST.s-0.5;
	xdif_sq = pow(xdif,2.0);
	ydif = vST.t-0.5;
	ydif_sq = pow(ydif,2.0);
	sum = xdif_sq + ydif_sq;
	dist = pow(sum,0.5);
	if( dist <= 0.1 * (AMP*sin(uTime)+0.2)) {myColor = vec3( 1., 0., 0. );}

	xdif = vST.s-0.8333333;
	xdif_sq = pow(xdif,2.0);
	ydif = vST.t-0.5;
	ydif_sq = pow(ydif,2.0);
	sum = xdif_sq + ydif_sq;
	dist = pow(sum,0.5);
	if( dist <= 0.1 * (AMP*sin(uTime)+0.2)) {myColor = vec3( 1., 0., 0. );}

	if( vST.t <= 0.2*(AMP*sin(uTime)+1) ||  vST.t >= 0.8*(AMP*sin(uTime)+1)){myColor = vec3( 1., 0., 0. );}

	gl_FragColor = vec4( myColor, 1. );
}