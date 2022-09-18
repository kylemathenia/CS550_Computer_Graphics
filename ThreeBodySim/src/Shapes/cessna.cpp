#pragma once
#include <stdio.h>
#include <math.h>
#include "glew.h"
#include <GL/gl.h>
#include "cessnaPts.h"
#include "../utils.h"
#include "../color.h"

#define PROPELLER_RADIUS	 1.0
#define PROPELLER_WIDTH		 0.4

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

void blade()
{
	glBegin(GL_TRIANGLES);
	glVertex2f(PROPELLER_RADIUS, PROPELLER_WIDTH / 2.);
	glVertex2f(0., 0.);
	glVertex2f(PROPELLER_RADIUS, -PROPELLER_WIDTH / 2.);

	glVertex2f(-PROPELLER_RADIUS, -PROPELLER_WIDTH / 2.);
	glVertex2f(0., 0.);
	glVertex2f(-PROPELLER_RADIUS, PROPELLER_WIDTH / 2.);
	glEnd();
}

GLuint
getBladeList()
{
	GLuint obj_list = glGenLists(1);
	glNewList(obj_list, GL_COMPILE);
	blade();
	glEndList();
	return obj_list;
}