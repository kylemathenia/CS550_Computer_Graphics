#pragma once

// which projection:
enum Projections
{
	ORTHO,
	PERSP
};

// which button:
enum ButtonVals
{
	RESET,
	SOFT_RESET,
	QUIT
};

// which view:
enum Views
{
	CENTER,
	BODY1,
	BODY2,
	BODY3,
	MAX_NUM_VIEWS = BODY3
};

// which view:
enum Tails
{
	CONST_THICK_LINE,
	VAR_THICK_LINE,
	CYLINDERS,
	SPHERES,
	NONE
};