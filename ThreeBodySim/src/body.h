#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>
#include <tgmath.h>

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
		lineVec = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
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
		lineList = getLineList();
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

		// Tail using tranformations. Slower, but maybe can do cool things this way. Fading line towards end. 
		// Could instead use cylinders...
		for (int i = 1; i < tailLen_i; i++) {
			static float maxAlpha = 1.0f;
			float alpha = ((float)(tailLen - i) / (float)tailLen) * maxAlpha;
			Eigen::Vector3f curPt = tail[i];
			Eigen::Vector3f nextPt = tail[i-1];
			float dist = findDist(tail[i], tail[i - 1]);
			Eigen::Vector3f dif = nextPt - curPt;
			Eigen::Vector3f rotAxis = findCrossProduct(lineVec, dif);
			float ang = findAngDotProduct(dif,lineVec);
			dx = (GLfloat)tail[i](0) - translation(0);
			dy = (GLfloat)tail[i](1) - translation(1);
			dz = (GLfloat)tail[i](2) - translation(2);
			glPushMatrix();
			glEnable(GL_BLEND);
			glColor4f(Colors[bcolor][0], Colors[bcolor][1], Colors[bcolor][2], alpha);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glTranslatef(dx, dy, dz);
			glRotatef(-ang,rotAxis(0), rotAxis(1), rotAxis(2));
			glScalef(1, dist, 1);
			glCallList(lineList);
			glPopMatrix();
		}

		//// Sphere tails
		//for (int i = 0; i < tailLen_i; i++) {
		//	dx = (GLfloat)tail[i](0) - translation(0);
		//	dy = (GLfloat)tail[i](1) - translation(1);
		//	dz = (GLfloat)tail[i](2) - translation(2);
		//	glPushMatrix();
		//	glEnable(GL_BLEND);
		//	glColor4f(Colors[bcolor][0], Colors[bcolor][1], Colors[bcolor][2], 0.3f);
		//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//	glTranslatef(dx, dy, dz);
		//	glScalef(0.5f, 0.5f, 0.5f);
		//	glColor3fv(&Colors[bcolor][0]);
		//	glCallList(sphereList);
		//	glPopMatrix();
		//}

		//// Tail
		//glPushMatrix();
		//glTranslatef(-translation(0), -translation(1), -translation(2));
		//glCallList(tailList);
		//glPopMatrix();
	}

	void updateTail()
	{
		// Shift the array and add one. 
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
		glLineWidth((GLfloat)1.5f);
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

	GLuint getLineList()
	{
		GLuint obj_list = glGenLists(1);
		glNewList(obj_list, GL_COMPILE);
		glLineWidth((GLfloat)1.5f);
		glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(lineVec(0), lineVec(1), lineVec(2));
		glEnd();
		glEndList();
		return obj_list;
	}

	float findVolume()
	{
		return (4.0f * 3.14 * pow(r, 3)) / 3.0f;
	}

	float findDist(Eigen::Vector3f p1, Eigen::Vector3f p2)
	{
		return sqrt(pow(p1(0) - p2(0), 2) + pow(p1(1) - p2(1), 2) + pow(p1(2) - p2(2), 2));
	}

	Eigen::Vector3f findCrossProduct(Eigen::Vector3f b, Eigen::Vector3f a)
	{
		// The rotation angle is the cross product.
		float i = (a(1) * b(2)) - (a(2) * b(1));
		float j = (a(0) * b(2)) - (a(2) * b(0));
		float k = (a(0) * b(1)) - (a(1) * b(0));
		return Eigen::Vector3f(i, j, k);
	}

	float findMag(Eigen::Vector3f x)
	{
		return sqrt(pow(x(0), 2) + pow(x(1), 2) + pow(x(2), 2));
	}

	float findAngCrossProduct(Eigen::Vector3f crossP, Eigen::Vector3f a, Eigen::Vector3f b)
	{
		float numer = findMag(crossP);
		float denom = findMag(a) * findMag(b);
		return (180.0f / 3.1416f) * asin(numer/denom);
	}

	float findAngDotProduct(Eigen::Vector3f a, Eigen::Vector3f b)
	{
		float numer = a(0)*b(2) + a(1)*b(1) + a(2)*b(0);
		float denom = findMag(a) * findMag(b);
		return (180.0f / 3.1416f) * acos(numer / denom);
	}

	int id;
	float r_i, m_i, r, m, V;
	long tailLen_i, tailLen;
	state S_i,S;
	Eigen::Vector3f* tail;
	GLuint sphereList, selectedSphereList, tailList, lineList;
	enum Colors bcolor;
	enum Colors tcolor;
	bool selected;
	Eigen::Vector3f lineVec;
};
