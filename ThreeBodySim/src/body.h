#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>

#include "color.h"
#include "Shapes/shapes.h"
#include "Eigen/Dense"

struct state {
	Eigen::Vector3f pos;
	Eigen::Vector3f vel;
	Eigen::Vector3f acc;
};

class Body
{
public:
	Body()
	{

	};

	Body(int id, float rad,float mass, long tailLen, state init_state, enum Colors bc, enum Colors tc)
	{
		id = id;
		r_i = rad;
		m_i = mass;
		tailLen_i = tailLen;
		tail = new Eigen::Vector3f[tailLen];
		S_i = init_state;
		bcolor = bc;
		tcolor = tc;
		hardReset();
	};

	void hardReset()
	{
		r = m_i;
		m = m_i;
		tailLen = tailLen_i;
		S = S_i;
		initTail();
	}

	void initTail()
	{
		// Tail is the last tailLen states of the body. 
		for (int i=0; i<tailLen;i++) {
			tail[i] = S.pos;
		}
	}

	void initList()
	{
		sphereList = getSphereList(r_i, 40, 40);
	}

	void draw(Eigen::Vector3f center)
	{
		GLfloat dx = (GLfloat)S.pos(0) - center(0);
		GLfloat dy = (GLfloat)S.pos(1) - center(1);
		GLfloat dz = (GLfloat)S.pos(2) - center(2);
		glPushMatrix();
		glTranslatef(dx,dy,dz);
		glColor3fv(&Colors[bcolor][0]);
		glCallList(sphereList);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-center(0), -center(1), -center(2));
		glColor3fv(&Colors[tcolor][0]);
		glCallList(tailList);
		glPopMatrix();
	}

	void updateTail()
	{
		// Shift the array and add one. You could probably do it an efficient way
		// by moving the pointer at the head of the array...but optimize later...
		Eigen::Vector3f temp;
		Eigen::Vector3f prev_pos = tail[0];
		for (int i = 1; i < tailLen_i; i++) {
			temp = tail[i];
			tail[i] = prev_pos;
			prev_pos = temp;
		}
		tail[0] = S.pos;
		getTailList();
	}

	void getTailList()
	{
		glDeleteLists(tailList, 1);
		GLuint obj_list = glGenLists(1);
		glNewList(obj_list, GL_COMPILE);
		glLineWidth((GLfloat)1.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < tailLen_i - 1; i++) {
			glVertex3f((GLfloat)tail[i](0), (GLfloat)tail[i](1), (GLfloat)tail[i](2));
			glVertex3f((GLfloat)tail[i + 1](0), (GLfloat)tail[i + 1](1), (GLfloat)tail[i + 1](2));
		}
		glEnd();
		glEndList();
		tailList = obj_list;
	}

	int id;
	float r_i, m_i, r, m;
	long tailLen_i, tailLen;
	state S_i,S;
	Eigen::Vector3f* tail;
	GLuint sphereList;
	GLuint tailList;
	enum Colors bcolor;
	enum Colors tcolor;
};
