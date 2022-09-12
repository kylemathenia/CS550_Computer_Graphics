#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "glew.h"
#include <GL/gl.h>


// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;
// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

// the stroke characters 'X' 'Y' 'Z' :
static float xx[] = {
		0.f, 1.f, 0.f, 1.f
};
static float xy[] = {
		-.5f, .5f, .5f, -.5f
};
static int xorder[] = {
		1, 2, -3, 4
};
static float yx[] = {
		0.f, 0.f, -.5f, .5f
};
static float yy[] = {
		0.f, .6f, 1.f, 1.f
};
static int yorder[] = {
		1, 2, 3, -2, 4
};
static float zx[] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
};
static float zy[] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
};
static int zorder[] = {
		1, 2, 3, 4, -5, 6
};

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();
}


GLuint
getAxesList(const GLfloat axesWidth)
{
	GLuint obj_list = glGenLists(1);
	glNewList(obj_list, GL_COMPILE);
	glLineWidth(axesWidth);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
	return obj_list;
}