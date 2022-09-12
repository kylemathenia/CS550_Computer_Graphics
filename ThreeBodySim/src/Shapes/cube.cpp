#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "glew.h"
#include <GL/gl.h>


void cube(float BOXSIZE)
{
	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;

	glBegin(GL_QUADS);
	glColor3f(0., 0., 1.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(-dx, dy, dz);

	glVertex3f(-dx, -dy, -dz);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, -dy, -dz);

	glColor3f(1., 0., 0.);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, dy, dz);

	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, dy, dz);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(-dx, -dy, -dz);

	glColor3f(0., 1., 0.);
	glVertex3f(-dx, dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(-dx, dy, -dz);

	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, -dy, -dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, -dy, dz);

	glEnd();
}

GLuint
getCubeList(float BOXSIZE)
{
	GLuint obj_list = glGenLists(1);
	glNewList(obj_list, GL_COMPILE);
	cube(BOXSIZE);
	glEndList();
	return obj_list;
}