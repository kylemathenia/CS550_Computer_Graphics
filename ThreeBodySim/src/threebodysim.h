#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>
#include <iostream>
#include <math.h>

#include "body.h"
#include "Eigen/Dense"

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
		speed = 1.0f;
		reset();
	};

	void reset()
	{
		updateCenter();
	}

	void step()
	{
		findAccels();
		updateTime();
		updateBodies();
		updateCenter();
	}

	void drawBodies()
	{
		// since we are using glScalef( ), be sure normals get unitized:
		glEnable(GL_NORMALIZE);
		b1.draw(center);
		b2.draw(center);
		b3.draw(center);
	}

	void initLists()
	{
		b1.initList();
		b2.initList();
		b3.initList();
	}

	void speedUp()
	{
		speed = speed * 2;
	}

	void slowDown()
	{
		speed = speed / 2;
	}

	double prevTime, dt;
	Eigen::Vector3f c;
	Body b1, b2, b3;
	Eigen::Vector3f center;
	float speed;

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