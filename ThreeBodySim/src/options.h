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
	
	LINES, 
	CYLINDERS,
	SPHERES,
	NONE,
};

// glut buttons:
enum EventEnums
{
	ESCAPE = 0x1b,
	SCROLL_WHEEL_UP = 3,
	SCROLL_WHEEL_DOWN = 4,
	SCROLL_WHEEL_CLICK_FACTOR = 5,
	LEFT = 4,
	MIDDLE = 2,
	RIGHT = 1
};