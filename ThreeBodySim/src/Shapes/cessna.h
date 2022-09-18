#pragma once
#include <stdio.h>
#define _USE_MATH_DEFINES
#include "glew.h"
#include <GL/gl.h>
#include "../color.h"
#include "../utils.h"
#include "../options.h"
#include "Eigen/Dense"
#include "cessnaPts.h"



#define PROPELLER_RADIUS	 1.0
#define PROPELLER_WIDTH		 0.4


void cessna(enum Colors c);
GLuint getCessnaList(enum Colors c);
void blade();
GLuint getBladeList();



class Cessna
{
public:
	Cessna(enum Colors color)
	{
		c = color;
	};

	void step()
	{
		updateTime();
		rotAngle = rotSpeed * time;
	}

	void initList()
	{
		cessnaList = getCessnaList(Colors::GREEN);
		bladeList = getBladeList();
	}

	void draw()
	{
		drawPlane();
		drawBlades();
	}

	void drawPlane()
	{
		glPushMatrix();

		//glRotatef(-7., 0., 1., 0.);
		//glTranslatef(0., -1., 0.);
		//glRotatef(97., 0., 1., 0.);
		//glRotatef(-15., 0., 0., 1.);

		glRotatef(90., 0., 1., 0.);
		glCallList(cessnaList);
		glPopMatrix();
	}

	void drawBlades()
	{
		drawMainProp();
		drawVertProps();
	}

	void drawMainProp()
	{
		glPushMatrix();
		glColor3f(Colors[Colors::WHITE][0], Colors[Colors::WHITE][1], Colors[Colors::WHITE][2]);
		glTranslatef(0, 0, 7.5f);
		glRotatef(rotAngle, 0., 0., 1.);
		glScalef(5, 5, 5);
		glCallList(bladeList);
		glPopMatrix();
	}

	void drawVertProps()
	{
		glPushMatrix();
		glColor3f(Colors[Colors::WHITE][0], Colors[Colors::WHITE][1], Colors[Colors::WHITE][2]);
		glTranslatef(10.0f, 3.0f, 0);
		glRotatef(rotAngle*2, 0., 1., 0.);
		glRotatef(90., 1., 0., 0.);
		glScalef(3, 3, 3);
		glCallList(bladeList);
		glPopMatrix();

		glPushMatrix();
		glColor3f(Colors[Colors::WHITE][0], Colors[Colors::WHITE][1], Colors[Colors::WHITE][2]);
		glTranslatef(-10.0f, 3.0f, 0);
		glRotatef(-rotAngle*2, 0., 1., 0.);
		glRotatef(90., 1., 0., 0.);
		glScalef(3, 3, 3);
		glCallList(bladeList);
		glPopMatrix();
	}


	void updateTime()
	{
		time = ((float)glutGet(GLUT_ELAPSED_TIME)) / 1000.f;
	}

	GLuint cessnaList, bladeList;
	enum Colors c;
	double time; // seconds
	float speed;
	float rotAngle = 0;
	double rotSpeed = 180.0f; // degrees per second.
};













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
