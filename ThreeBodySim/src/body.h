#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>
#include <tgmath.h>
#include "color.h"
#include "Shapes/shapes.h"
#include "Eigen/Dense"
#include "math_utils.h"

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

	Body(int id, float rad,float mass, int tailLen, state init_state, enum Colors bc, enum Colors tc,int fps)
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
		// This just happens to often be a good spacing. 
		tailSpacing = fps / 10;
		tailUpdateCount = -tailSpacing;
		lineVec = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
		lineListVec = new GLuint[tailLen];
		hardReset();
	};

	void hardReset()
	{
		static int resets = 0;
		r = r_i;
		m = m_i;
		tailLen = tailLen_i;
		S = state{ S_i.pos,S_i.vel,S_i.acc };
		V = findSphereVolume(r);
		if (resets > 0) { delLists(); }
		initLists();
		initTail();
		resets++;
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
		r += rad_change;
		V = findSphereVolume(r);
		float change_ratio = V / prev_V;
		m = m * change_ratio;
		delLists();
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

	void delLists()
	{
		glDeleteLists(sphereList,1);
		glDeleteLists(selectedSphereList, 1);
		glDeleteLists(coneList, 1);
		glDeleteLists(lineList, 1);
		delLineListVec();
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

	void delLineListVec()
	{
		for (int i = 0; i < tailLen_i; i++) {
			glDeleteLists(lineListVec[i],1);
		}
	}

	// ##################### RENDER FUNCTIONS ##################### //

	void draw(Eigen::Vector3f translation,Tails tailOption)
	{
		GLfloat dx = (GLfloat)S.pos(0) - translation(0);
		GLfloat dy = (GLfloat)S.pos(1) - translation(1);
		GLfloat dz = (GLfloat)S.pos(2) - translation(2);

		drawBody(dx, dy, dz);
		if (selected == true) { drawSelector(dx, dy, dz); }
		// Tail options
		if (tailOption == Tails::CONST_THICK_LINE){drawConstThickLineTail(translation, 1.0f, 1.5f);}
		else if (tailOption == Tails::VAR_THICK_LINE){drawVarThickLineTail(translation, 1.0f, 3.0f);}
		else if (tailOption == Tails::CYLINDERS){drawCylinderTail(translation, 1.0f, 0.1f);}
		else if (tailOption == Tails::SPHERES){drawSphereTail(translation, 0.5f, 0.5f, false);}
		else if (tailOption == Tails::SPHERES_AND_LINES) { 
			drawSphereTail(translation, 0.5f, 0.5f, false); 
			drawCylinderTail(translation, 0.8f, 0.1f);
		}
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
			ang = findAngDotProductD(dif, lineVec);
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
			ang = findAngDotProductD(dif, lineVec);
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
			ang = findAngDotProductD(dif, lineVec);
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
			percentComplete = ((float)(tailLen - (i+ tailSpacingOffset))) / (tailLen);
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

	int id;
	float r_i, m_i, r, m, V;
	long tailLen_i, tailLen;
	state S_i,S;
	Eigen::Vector3f* tail;
	GLuint sphereList, selectedSphereList, coneList, lineList;
	enum Colors bcolor;
	enum Colors tcolor;
	bool selected;
	Eigen::Vector3f lineVec, prevPos;
	GLuint* lineListVec;
	int tailSpacingOffset = 0;
	int tailSpacing;
	long long tailUpdateCount;
};
