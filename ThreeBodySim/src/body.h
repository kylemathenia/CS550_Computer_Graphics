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

struct tailPt {
	Eigen::Vector3f pos;
	GLuint list;
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
		selected = false;
		hardReset();
	};

	void hardReset()
	{
		r = r_i;
		m = m_i;
		tailLen = tailLen_i;
		S = state{ S_i.pos,S_i.vel,S_i.acc };
		V = findVolume();
		initLists();
		initTail();
	}

	void initTail()
	{
		// Tail is the last tailLen states of the body. 
		for (int i = 0; i < tailLen; i++) {
			tail[i] = S.pos;
		}
	}

	void changeSize(float rad_change)
	{
		if (r + rad_change < 0) { return; }
		float prev_V = V;
		r = r + rad_change;
		V = findVolume();
		float change_ratio = V / prev_V;
		m = m * change_ratio;
		initLists();
	}

	void initLists()
	{
		sphereList = getSphereList(r, 40, 40);
		selectedSphereList = getSphereList(r*1.5f, 40, 40);
	}

	void changeLists()
	{
		glDeleteLists(sphereList, 1);
		glDeleteLists(selectedSphereList, 1);
		initLists();
	}

	void draw(Eigen::Vector3f translation)
	{
		GLfloat dx = (GLfloat)S.pos(0) - translation(0);
		GLfloat dy = (GLfloat)S.pos(1) - translation(1);
		GLfloat dz = (GLfloat)S.pos(2) - translation(2);
		glPushMatrix();
		glTranslatef(dx,dy,dz);
		glColor3fv(&Colors[bcolor][0]);
		glCallList(sphereList);
		glPopMatrix();

		// If selected, draw a transparent sphere that is slightly larger. 
		if (selected == true) {
			glPushMatrix();
			glEnable(GL_BLEND);
			glColor4f(Colors[Colors::WHITE][0], Colors[Colors::WHITE][1], Colors[Colors::WHITE][2], 0.3f);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glTranslatef(dx, dy, dz);
			glCallList(selectedSphereList);
			glPopMatrix();
		}

		// Tail
		glPushMatrix();
		glTranslatef(-translation(0), -translation(1), -translation(2));
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
		glColor3fv(&Colors[tcolor][0]);
		glBegin(GL_LINES);
		for (int i = 0; i < tailLen_i - 1; i++) {
			glVertex3f((GLfloat)tail[i](0), (GLfloat)tail[i](1), (GLfloat)tail[i](2));
			glVertex3f((GLfloat)tail[i + 1](0), (GLfloat)tail[i + 1](1), (GLfloat)tail[i + 1](2));
		}
		glEnd();
		glEndList();
		tailList = obj_list;
	}

	float findVolume()
	{
		return (4.0f * 3.14 * pow(r, 3)) / 3.0f;
	}

	int id;
	float r_i, m_i, r, m, V;
	long tailLen_i, tailLen;
	state S_i,S;
	Eigen::Vector3f* tail;
	//tailPt* tail;
	GLuint sphereList;
	GLuint selectedSphereList;
	GLuint tailList;
	enum Colors bcolor;
	enum Colors tcolor;
	bool selected;
};
