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

	void draw(pt3i translation)
	{
		glPushMatrix();
		glTranslatef((GLfloat)translation.x, translation.y, translation.z);
		glColor3fv(&Colors[Colors::WHITE][0]);
		glCallList(axisList);
		glPopMatrix();
	}

	float r_i, m_i, r, m;
	GLuint axisList;
	GLfloat axisWidth;
};