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

	void draw(GLfloat Yrot, GLfloat Xrot)
	{
		glPushMatrix();
		glTranslatef((GLfloat)-8, -8, -2);
		glRotatef((GLfloat)Yrot, 0., 1., 0.);
		glRotatef((GLfloat)Xrot, 1., 0., 0.);
		glColor3fv(&Colors[Colors::WHITE][0]);
		glCallList(axisList);
		glPopMatrix();
	}

	float r_i, m_i, r, m;
	GLuint axisList;
	GLfloat axisWidth;
};