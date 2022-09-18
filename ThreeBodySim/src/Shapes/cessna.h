#pragma once
#include <stdio.h>
#define _USE_MATH_DEFINES
#include "glew.h"
#include <GL/gl.h>
#include "../color.h"
#include "../utils.h"
#include "cessnaPts.h"



void cessna(enum Colors c)
{
	int i;
	struct point* p0, * p1, * p2;
	struct tri* tp;
	float p01[3], p02[3], n[3];

	glBegin(GL_TRIANGLES);
	for (i = 0, tp = CESSNAtris; i < CESSNAntris; i++, tp++)
	{
		p0 = &CESSNApoints[tp->p0];
		p1 = &CESSNApoints[tp->p1];
		p2 = &CESSNApoints[tp->p2];

		// fake "lighting" from above:
		p01[0] = p1->x - p0->x;
		p01[1] = p1->y - p0->y;
		p01[2] = p1->z - p0->z;
		p02[0] = p2->x - p0->x;
		p02[1] = p2->y - p0->y;
		p02[2] = p2->z - p0->z;
		Cross(p01, p02, n);
		Unit(n, n);
		n[1] = fabs(n[1]);
		glColor3f(Colors[c][0] * n[1], Colors[c][1] * n[1], Colors[c][2] * n[1]);

		glVertex3f(p0->x, p0->y, p0->z);
		glVertex3f(p1->x, p1->y, p1->z);
		glVertex3f(p2->x, p2->y, p2->z);
	}
	glEnd();
}

GLuint
getCessnaList(enum Colors c)
{
	GLuint obj_list = glGenLists(1);
	glNewList(obj_list, GL_COMPILE);
	cessna(c);
	glEndList();
	return obj_list;
}

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