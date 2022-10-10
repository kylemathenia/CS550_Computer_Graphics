#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>
#include "Shapes/shapes.h"
#include "color.h"
#include "utils.h"

class Axis
{
public:
	Axis()
	{

	};

	Axis(const GLfloat width)
	{
		axisWidth = width;
		hardReset();
	};

	void hardReset()
	{

	}

	void initList()
	{
		axisList = getAxesList(axisWidth);
	}

	void draw(pt2f rot, pt3i translation)
	{
		glPushMatrix();
		glTranslatef((GLfloat)translation.x, translation.y, translation.z);
		glRotatef((GLfloat)rot.y, 0., 1., 0.);
		glRotatef((GLfloat)rot.x, 1., 0., 0.);
		glColor3fv(&ColorsArr[Colors::WHITE][0]);
		glCallList(axisList);
		glPopMatrix();
	}

	float r_i, m_i, r, m;
	GLuint axisList;
	GLfloat axisWidth;
};