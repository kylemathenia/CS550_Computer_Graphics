#pragma once
#include "cessnaPts.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include "glew.h"
#include <GL/gl.h>
#include "../color.h"
#include "../utils.h"

void cessna(enum Colors c);
GLuint getCessnaList(enum Colors c);

class Cessna
{
public:
	Cessna(enum Colors color)
	{
		c = color;
	};

	void initList()
	{
		cessnaList = getCessnaList(Colors::GREEN);
	}

	void draw()
	{
		glPushMatrix();
		glPushMatrix();
		glRotatef(-7., 0., 1., 0.);
		glTranslatef(0., -1., 0.);
		glRotatef(97., 0., 1., 0.);
		glRotatef(-15., 0., 0., 1.);
		glCallList(cessnaList);
		glPopMatrix();
	}

	GLuint cessnaList;
	enum Colors c;
};