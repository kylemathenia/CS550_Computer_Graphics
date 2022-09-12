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
		tail = new tailPt[tailLen];
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
		// I don't think this works because we didn't do a deep copy. 
		S = S_i;
		V = findVolume();
		initTail();
	}

	void initTail()
	{
		// Tail is the last tailLen states of the body. 
		for (int i = 0; i < tailLen; i++) {
			tail[i].pos = S.pos;
			tail[i].list = getTailSegment(S.pos, S.pos);
		}
	}

	void changeSize(float rad_change)
	{
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

		for (int i = 0; i < tailLen_i; i++) {
			glPushMatrix();
			glTranslatef(-center(0), -center(1), -center(2));
			glColor3fv(&Colors[tcolor][0]);
			glCallList(tail[i].list);
			glPopMatrix();
		}

	}

	void updateTail()
	{
		// Shift the array and add one. You could probably do it an efficient way
		// by moving the pointer at the head of the array...but optimize later...
		Eigen::Vector3f temp_pos;
		GLuint temp_list;
		Eigen::Vector3f prev_pos = tail[0].pos;
		GLuint prev_list = tail[0].list;
		for (int i = 1; i < tailLen_i-1; i++) {
			temp_pos = tail[i].pos;
			temp_list = tail[i].list;
			tail[i].pos = prev_pos;
			tail[i].list = prev_list;
			prev_pos = temp_pos;
			prev_list = temp_list;
		}
		// Don't create memory leaks when replacing the end of the list. 
		tail[tailLen_i - 1].pos = prev_pos;
		temp_list = tail[tailLen_i - 1].list;
		tail[tailLen_i - 1].list = prev_list;
		glDeleteLists(temp_list, 1);

		tail[0].pos = S.pos;
		tail[0].list = getTailSegment(tail[0].pos, tail[1].pos);
	}

	GLuint getTailSegment(Eigen::Vector3f pos1, Eigen::Vector3f pos2)
	{
		GLuint obj_list = glGenLists(1);
		glNewList(obj_list, GL_COMPILE);
		glLineWidth((GLfloat)1.0f);
		glBegin(GL_LINES);
		glVertex3f((GLfloat)pos1(0), (GLfloat)pos1(1), (GLfloat)pos1(2));
		glVertex3f((GLfloat)pos2(0), (GLfloat)pos2(1), (GLfloat)pos2(2));
		glEnd();
		glEndList();
		return obj_list;
	}

	float findVolume()
	{
		return (4.0f * 3.14 * pow(r, 3)) / 3.0f;
	}

	int id;
	float r_i, m_i, r, m, V;
	long tailLen_i, tailLen;
	state S_i,S;
	tailPt* tail;
	GLuint sphereList;
	GLuint selectedSphereList;
	GLuint tailList;
	enum Colors bcolor;
	enum Colors tcolor;
	bool selected;
};
