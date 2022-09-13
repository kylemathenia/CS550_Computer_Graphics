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
		lineListVec = new GLuint[tailLen];
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
		coneList = getConeList(1, 1, 1, 15, 15,false,false);
		lineList = getLineList(1.5f);
		getLineListVec(3.0f);
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
		tailSpacingOffset++;
		if (tailSpacingOffset % tailSpacing == 0) { tailSpacingOffset = 0; }
		tailUpdateCount++;
	}

	GLuint getLineList(float width)
	{
		GLuint obj_list = glGenLists(1);
		glNewList(obj_list, GL_COMPILE);
		glLineWidth((GLfloat)width);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(lineVec(0), lineVec(1), lineVec(2));
		glEnd();
		glEndList();
		return obj_list;
	}

	void getLineListVec(float maxWidth)
	{
		for (int i = 0; i < tailLen_i; i++) {
			float percentComplete = ((float)(tailLen - i) / (float)tailLen);
			float width = percentComplete * maxWidth;
			lineListVec[i] = getLineList(width);
		}
	}

	// ##################### RENDER FUNCTIONS ##################### //

	void draw(Eigen::Vector3f translation)
	{
		GLfloat dx = (GLfloat)S.pos(0) - translation(0);
		GLfloat dy = (GLfloat)S.pos(1) - translation(1);
		GLfloat dz = (GLfloat)S.pos(2) - translation(2);

		drawBody(dx, dy, dz);
		if (selected == true) { drawSelector(dx, dy, dz); }
		//drawConstThickLineTail(translation, 1.0f,1.5f);
		//drawVarThickLineTail(translation, 1.0f, 3.0f);
		//drawCylinderTail(translation, 1.0f, 0.1f);
		drawSphereTail(translation, 0.5f, 0.5f,false);
	}

	void drawBody(GLfloat dx, GLfloat dy, GLfloat dz)
	{
		glPushMatrix();
		glTranslatef(dx, dy, dz);
		glColor3fv(&Colors[bcolor][0]);
		glCallList(sphereList);
		glPopMatrix();
	}

	void drawSelector(GLfloat dx, GLfloat dy, GLfloat dz)
	{
		// If selected, draw a transparent sphere that is slightly larger. 
		glPushMatrix();
		glEnable(GL_BLEND);
		glColor4f(Colors[Colors::WHITE][0], Colors[Colors::WHITE][1], Colors[Colors::WHITE][2], 0.3f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glTranslatef(dx, dy, dz);
		glCallList(selectedSphereList);
		glPopMatrix();
	}

	void drawConstThickLineTail(Eigen::Vector3f translation, float maxAlpha,float width)
	{
		// Tail using tranformations with const thickness lines, fading with length. 
		glDeleteLists(lineList, 1);
		lineList = getLineList(width);
		float alpha, dist, ang;
		GLfloat dx, dy, dz;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis;
		for (int i = 1; i < tailLen_i; i++) {
			alpha = ((float)(tailLen - i) / (float)tailLen) * maxAlpha;
			curPt = tail[i];
			nextPt = tail[i - 1];
			dist = findDist(tail[i], tail[i - 1]);
			dif = nextPt - curPt;
			rotAxis = findCrossProduct(lineVec, dif);
			ang = findAngDotProduct(dif, lineVec);
			dx = (GLfloat)tail[i](0) - translation(0);
			dy = (GLfloat)tail[i](1) - translation(1);
			dz = (GLfloat)tail[i](2) - translation(2);
			glPushMatrix();
			glEnable(GL_BLEND);
			glColor4f(Colors[bcolor][0], Colors[bcolor][1], Colors[bcolor][2], alpha);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glTranslatef(dx, dy, dz);
			glRotatef(-ang, rotAxis(0), rotAxis(1), rotAxis(2));
			glScalef(100, dist, 100);
			glCallList(lineList);
			glPopMatrix();
		}
	}

	void drawVarThickLineTail(Eigen::Vector3f translation, float maxAlpha, float maxWidth)
	{
		// Tail using tranformations with lines. Changing the thickness of the line, fading with length.
		float alpha, dist, ang,width, percentComplete;
		GLfloat dx, dy, dz;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis;
		for (int i = 1; i < tailLen_i; i++) {
			percentComplete = ((float)(tailLen - i) / (float)tailLen);
			alpha = percentComplete * maxAlpha;
			width = percentComplete * maxWidth;
			curPt = tail[i];
			nextPt = tail[i - 1];
			dist = findDist(tail[i], tail[i - 1]);
			dif = nextPt - curPt;
			rotAxis = findCrossProduct(lineVec, dif);
			ang = findAngDotProduct(dif, lineVec);
			dx = (GLfloat)tail[i](0) - translation(0);
			dy = (GLfloat)tail[i](1) - translation(1);
			dz = (GLfloat)tail[i](2) - translation(2);
			glPushMatrix();
			glEnable(GL_BLEND);
			glColor4f(Colors[bcolor][0], Colors[bcolor][1], Colors[bcolor][2], alpha);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glTranslatef(dx, dy, dz);
			glRotatef(-ang, rotAxis(0), rotAxis(1), rotAxis(2));
			glScalef(100, dist, 100);
			glCallList(lineListVec[i]);
			glPopMatrix();
		}
	}

	void drawCylinderTail(Eigen::Vector3f translation, float maxAlpha, float maxScale)
	{
		//Tail using tranformations with cylinders, getting smaller and fading with length. 
		float alpha, dist, ang, scale, percentComplete;
		GLfloat dx, dy, dz;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis;
		for (int i = 1; i < tailLen_i; i++) {
			percentComplete = ((float)(tailLen - i) / (float)tailLen);
			alpha = percentComplete * maxAlpha;
			scale = percentComplete * maxScale;
			curPt = tail[i];
			nextPt = tail[i - 1];
			dist = findDist(tail[i], tail[i - 1]);
			dif = nextPt - curPt;
			rotAxis = findCrossProduct(lineVec, dif);
			ang = findAngDotProduct(dif, lineVec);
			dx = (GLfloat)tail[i](0) - translation(0);
			dy = (GLfloat)tail[i](1) - translation(1);
			dz = (GLfloat)tail[i](2) - translation(2);
			glPushMatrix();
			glEnable(GL_BLEND);
			glColor4f(Colors[bcolor][0], Colors[bcolor][1], Colors[bcolor][2], alpha);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glTranslatef(dx, dy, dz);
			glRotatef(-ang, rotAxis(0), rotAxis(1), rotAxis(2));
			glScalef(scale, dist * 1.01f, scale);
			glCallList(coneList);
			glPopMatrix();
		}
	}

	void drawSphereTail(Eigen::Vector3f translation, float maxAlpha, float maxScale,bool gettingSmaller)
	{
		// Sphere tails, getting smaller and more transparent.
		float alpha, dist, ang, scale, percentComplete;
		GLfloat dx, dy, dz;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis;
		int j;
		for (int i = 0; i < tailLen_i - tailSpacing; i += tailSpacing) {
			percentComplete = ((float)(tailLen - i)) / (tailLen);
			alpha = percentComplete * maxAlpha;
			if (gettingSmaller == true) { scale = percentComplete * maxScale; }
			else { scale = maxScale; }
			j = i + tailSpacingOffset;
			// Don't draw the first sphere because there is an ugly gap. 
			if (i > tailUpdateCount) { break; }
			dx = (GLfloat)tail[j](0) - translation(0);
			dy = (GLfloat)tail[j](1) - translation(1);
			dz = (GLfloat)tail[j](2) - translation(2);
			glPushMatrix();
			glEnable(GL_BLEND);
			glColor4f(Colors[bcolor][0], Colors[bcolor][1], Colors[bcolor][2], alpha);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glTranslatef(dx, dy, dz);
			glScalef(scale, scale, scale);
			glCallList(sphereList);
			glPopMatrix();
		}
	}

	// ##################### MATH FUNCTIONS ##################### //

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
	GLuint sphereList, selectedSphereList, coneList, lineList;
	enum Colors bcolor;
	enum Colors tcolor;
	bool selected;
	Eigen::Vector3f lineVec;
	GLuint* lineListVec;
	int tailSpacingOffset = 0;
	int tailSpacing = 6;
	long long tailUpdateCount = -tailSpacing;
};
