#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>
#include <iostream>
#include <math.h>

#include "body.h"
#include "Eigen/Dense"
#include "options.h"


class ThreeBodySim
{
public:
	ThreeBodySim(Body& body1, Body& body2, Body& body3)
	{
		b1 = body1;
		b2 = body2;
		b3 = body3;
		prevTime = 0;
		dt = 0;
		reset();
	};

	void reset()
	{
		b1.hardReset();
		b2.hardReset();
		b3.hardReset();
		updateCenter();
		speed = 1.0f;
		bufferChangeCount = 0;
		changeSelected(0);
	}

	void step()
	{
		findAccels();
		updateTime();
		updateBodies();
		updateCenter();
	}

	void drawBodies(Views view,Tails tailOption)
	{
		Eigen::Vector3f translation;
		if (view == Views::CENTER) { translation = center; }
		else if (view == Views::BODY1) { translation = b1.S.pos; }
		else if (view == Views::BODY2) { translation = b2.S.pos; }
		else { translation = b3.S.pos; }
		// since we are using glScalef( ), be sure normals get unitized:
		glEnable(GL_NORMALIZE);
		b1.draw(translation, tailOption);
		b2.draw(translation, tailOption);
		b3.draw(translation, tailOption);
	}

	void initLists()
	{
		b1.initLists();
		b2.initLists();
		b3.initLists();
	}

	void changeSpeed(float multiplier)
	{
		speed = speed * multiplier;
	}

	void changeSize(float rad_change)
	{
		for (int i = 0; i < 3; i++) {
			if (bodyList[i]->selected == true)
			{
				bodyList[i]->changeSize(rad_change);
			}
		}
	}

	// Psuedo circular buffer.
	void changeSelected(int change)
	{
		bufferChangeCount+= change;
		int num = bufferChangeCount % (Views::MAX_NUM_VIEWS+1);
		if (num < 0) { num = num * -1; }
		if (num == 0) {
			b1.selected = false;
			b2.selected = false;
			b3.selected = false;
		}
		else if (num == 1) {
			b1.selected = true;
			b2.selected = false;
			b3.selected = false;
		}
		else if (num == 2) {
			b1.selected = false;
			b2.selected = true;
			b3.selected = false;
		}
		else if (num == 3) {
			b1.selected = false;
			b2.selected = false;
			b3.selected = true;
		}
	}

	double prevTime, dt;
	Eigen::Vector3f c;
	Body b1, b2, b3;
	Body* bodyList[3] = { &b1,&b2,&b3 };
	Eigen::Vector3f center;
	float speed;
	int bufferChangeCount;

private:
	void findAccels()
	{
		b1.S.acc = (numerator(b1, b2) / denominator(b1, b2)) + (numerator(b1, b3) / denominator(b1, b3));
		b2.S.acc = (numerator(b2, b3) / denominator(b2, b3)) + (numerator(b2, b1) / denominator(b2, b1));
		b3.S.acc = (numerator(b3, b1) / denominator(b3, b1)) + (numerator(b3, b2) / denominator(b3, b2));
	}

	void updateBodies()
	{
		b1.S.vel = b1.S.vel + (b1.S.acc * dt * speed);
		b2.S.vel = b2.S.vel + (b2.S.acc * dt * speed);
		b3.S.vel = b3.S.vel + (b3.S.acc * dt * speed);

		b1.S.pos = b1.S.pos + (b1.S.vel * dt * speed);
		b2.S.pos = b2.S.pos + (b2.S.vel * dt * speed);
		b3.S.pos = b3.S.pos + (b3.S.vel * dt * speed);

		b1.updateTail();
		b2.updateTail();
		b3.updateTail();

	}

	void updateCenter()
	{
		center = (b1.S.pos + b2.S.pos + b3.S.pos) / 3;
	}

	void updateTime()
	{
		float curTime = getCurTime();
		dt = (curTime - prevTime);
		prevTime = curTime;
	}

	void initTime() {while (getCurTime() > 0.1) {};}

	float getCurTime()
	{
		return ((float)glutGet(GLUT_ELAPSED_TIME)) / 1000.f;
	}

	Eigen::Vector3f numerator(Body a, Body b)
	{
		return -9.81 * b.m * (a.S.pos - b.S.pos);
	}

	double diffSquared(Body a, Body b, int i)
	{
		return std::pow(a.S.pos(i) - b.S.pos(i), 2.0);;
	}

	double denominator(Body a, Body b)
	{
		return std::pow(sqrt(diffSquared(a, b, 0) + diffSquared(a, b, 1) + diffSquared(a, b, 2)), 3);
	}

};