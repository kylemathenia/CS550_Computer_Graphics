#pragma once

enum Projections
{
	ORTHO,
	PERSP
};

enum ButtonVals
{
	RESET,
	SOFT_RESET,
	QUIT
};

enum Views
{
	CENTER,
	BODY1,
	BODY2,
	BODY3,
	MAX_NUM_VIEWS = BODY3
};

enum class Selections
{
	B1,
	B2,
	B3,
	ALL,
	NONE,
	MAX_NUM_SELECTIONS = NONE
};

enum class Bodies
{
	B1,
	B2,
	B3,
	BOUNDARY
};

enum Tails
{	
	LINES, 
	CYLINDERS,
	SPHERES,
	NONE
};

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