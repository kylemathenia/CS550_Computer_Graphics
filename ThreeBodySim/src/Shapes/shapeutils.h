#pragma once
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "glew.h"
#include <GL/gl.h>

struct point
{
	float x, y, z;		// coordinates
	float nx, ny, nz;	// surface normal
	float s, t;		// texture coords
};

inline
void
DrawPoint(struct point* p)
{
	glNormal3fv(&p->nx);
	glTexCoord2fv(&p->s);
	glVertex3fv(&p->x);
}