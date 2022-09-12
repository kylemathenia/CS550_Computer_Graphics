#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>

#include "Shapes/shapes.h"
#include "color.h"

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

	void draw()
	{
		glPushMatrix();
		/*glTranslatef((GLfloat)S.pos(0), S.pos(1), S.pos(2));*/
		glColor3fv(&Colors[Colors::WHITE][0]);
		glCallList(axisList);
		glPopMatrix();
	}

	float r_i, m_i, r, m;
	GLuint axisList;
	GLfloat axisWidth;
};