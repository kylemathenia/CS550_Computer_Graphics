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

	Body(Bodies bID, float rad,float mass, int tailLen, state init_state, enum Colors bc, enum Colors tc,int fps)
	{
		bType = bID;
		r_i = rad;
		m_i = mass;
		tailLen_i = tailLen;
		tail = new Eigen::Vector3f[tailLen];
		S_i = init_state;
		bcolor = bc;
		tcolor = tc;
		selected = false;
		// This just happens to often be a good spacing for the sphere tail most of the time. 
		tailSpacing = fps / 5;
		tailUpdateCount = -tailSpacing;
		lineVec = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
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
		for (int i = 0; i < tailLen; i++) {
			tail[i] = S.pos;
		}
	}

	void changeSize(float rad_change)
	{
		float newR = r * rad_change;
		if (newR < bodyMinRad) { return; }
		float prev_V = V;
		r = newR;
		V = findSphereVolume(r);
		float change_ratio = V / prev_V;
		m = m * change_ratio;
		delLists();
		initLists();
	}

	void initLists()
	{
		sphereList = getSphereList(abs(r), 40, 40);
		cylinderList = getConeList(1, 1, 1, 15, 15,false,false);
		lineList = getLineList(1.5f);
	}

	void delLists()
	{
		glDeleteLists(sphereList,1);
		glDeleteLists(cylinderList, 1);
		glDeleteLists(lineList, 1);
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


	// ##################### RENDER FUNCTIONS ##################### //

	void drawObliq(Eigen::Vector3f translation)
	{
		Eigen::Vector3f delta = S.pos - translation;
		Eigen::Vector3f scale = { 1, 1, 1 };
		Eigen::Vector3f rotAxis = { 1, 0, 0 };
		float ang = 0;
		drawGlSeqOpaq(sphereList, scale, delta, rotAxis, ang, bcolor);
	}

	void drawTran(Eigen::Vector3f translation, Tails tailOption)
	{
		if (selected == true) { drawSelector(translation); }
		if (tailOption == Tails::LINES) { drawLineTail(translation, 1.0f, 1.5f); }
		else if (tailOption == Tails::CYLINDERS) { drawCylinderTail(translation, 1.0f, 0.1f); }
		else if (tailOption == Tails::SPHERES) { drawSphereTail(translation, 0.3f, 0.5f, false); }
	}

	void drawSelector(Eigen::Vector3f translation)
	{
		Eigen::Vector3f delta = S.pos - translation;
		Eigen::Vector3f scale = { selectorScale, selectorScale, selectorScale };
		Eigen::Vector3f rotAxis = { 1, 0, 0 };
		float ang = 0;
		drawGlSeqTran(sphereList, scale, delta, rotAxis, ang, 0.3f, Colors::WHITE);
	}

	void drawBoundary(Eigen::Vector3f translation,float timeSinceContact)
	{
		float visDuration = 0.5;
		float minAlpha = 0.02;
		float maxAlpha = 0.15;
		float alpha = (((maxAlpha - minAlpha) * (visDuration - timeSinceContact)) / visDuration) + minAlpha;
		if (timeSinceContact > visDuration) { alpha = minAlpha; }
		Eigen::Vector3f delta = S.pos - translation;
		Eigen::Vector3f scale = { 1, 1, 1 };
		Eigen::Vector3f rotAxis = { 1, 0, 0 };
		float ang = 0;
		drawGlSeqTran(sphereList, scale, delta, rotAxis, ang, alpha, Colors::WHITE);
	}

	void drawLineTail(Eigen::Vector3f translation, float maxAlpha,float width)
	{
		// Be able to change the width of the line.
		glDeleteLists(lineList, 1);
		lineList = getLineList(width);
		float alpha, dist, ang;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis,delta,scale;
		for (int i = 1; i < tailLen_i; i++) {
			alpha = ((float)(tailLen - i) / (float)tailLen) * maxAlpha;
			curPt = tail[i];
			nextPt = tail[i - 1];
			dist = findDist(tail[i], tail[i - 1]);
			dif = nextPt - curPt;
			rotAxis = findCrossProduct(lineVec, dif);
			ang = -findAngDotProductD(dif, lineVec);
			delta = tail[i] - translation;
			scale = { 1,dist,1 };
			drawGlSeqTran(lineList, scale, delta, rotAxis, ang, alpha, bcolor);
		}
	}

	void drawCylinderTail(Eigen::Vector3f translation, float maxAlpha, float maxScale)
	{
		//Tail using tranformations with cylinders, getting smaller and fading with length. 
		float alpha, dist, ang, scaleMod, percentComplete;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis, delta, scale;
		for (int i = 1; i < tailLen_i; i++) {
			percentComplete = ((float)(tailLen - i) / (float)tailLen);
			alpha = percentComplete * maxAlpha;
			scaleMod = percentComplete * maxScale;
			curPt = tail[i];
			nextPt = tail[i - 1];
			dist = findDist(tail[i], tail[i - 1]);
			dif = nextPt - curPt;
			rotAxis = findCrossProduct(lineVec, dif);
			ang = -findAngDotProductD(dif, lineVec);
			delta = tail[i] - translation;
			scale = { r * scaleMod,dist * 1.01f,r * scaleMod };
			drawGlSeqTran(cylinderList, scale, delta, rotAxis, ang, alpha, bcolor);
		}
	}

	void drawSphereTail(Eigen::Vector3f translation, float maxAlpha, float maxScale,bool gettingSmaller)
	{
		// Sphere tails, getting smaller and more transparent.
		float alpha, uniformScale, percentComplete;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis, delta, scale;
		int j;
		for (int i = 0; i < tailLen_i - tailSpacing; i += tailSpacing) {
			percentComplete = ((float)(tailLen - (i+ tailSpacingOffset))) / (tailLen);
			alpha = percentComplete * maxAlpha;
			if (gettingSmaller == true) { uniformScale = percentComplete * maxScale; }
			else { uniformScale = maxScale; }
			scale = { uniformScale ,uniformScale ,uniformScale };
			j = i + tailSpacingOffset;
			// Don't draw the first sphere because there is an ugly gap. 
			if (i > tailUpdateCount) { break; }
			delta = (tail[j] - translation);
			drawGlSeqTran(sphereList, scale, delta, Eigen::Vector3f{1,0,0}, 0, alpha,bcolor);
		}
	}


	void drawGlSeqOpaq(GLuint list, Eigen::Vector3f scale, Eigen::Vector3f d, Eigen::Vector3f rotAxis, float ang, enum Colors c)
	{
		glPushMatrix();
		glEnable(GL_DEPTH_TEST);
		glColor3f(Colors[c][0], Colors[c][1], Colors[c][2]);
		drawGlSeq(list, scale, d, rotAxis, ang);
	}

	void drawGlSeqTran(GLuint list, Eigen::Vector3f scale, Eigen::Vector3f d, Eigen::Vector3f rotAxis, float ang, float alpha, enum Colors c)
	{
		glPushMatrix();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glColor4f(Colors[c][0], Colors[c][1], Colors[c][2], alpha);
		drawGlSeq(list, scale, d, rotAxis, ang);
	}

	void drawGlSeq(GLuint list, Eigen::Vector3f scale, Eigen::Vector3f d, Eigen::Vector3f rotAxis, float ang)
	{
		glTranslatef(d(0), d(1), d(2));
		glRotatef(ang, rotAxis(0), rotAxis(1), rotAxis(2));
		glScalef(scale(0), scale(1), scale(2));
		glCallList(list);
		glPopMatrix();
	}


	Bodies bType;
	float r_i, m_i, r, m;
	double V;
	long tailLen_i, tailLen;
	state S_i,S;
	Eigen::Vector3f* tail;
	GLuint sphereList, cylinderList, lineList;
	enum Colors bcolor;
	enum Colors tcolor;
	bool selected;
	Eigen::Vector3f lineVec, prevPos;
	GLuint* lineListVec;
	int tailSpacingOffset = 0;
	int tailSpacing;
	long long tailUpdateCount;
	float selectorScale = 1.1f;
	float bodyMinRad = 0.1f;
};
