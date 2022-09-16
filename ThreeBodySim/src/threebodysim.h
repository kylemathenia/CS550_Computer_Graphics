#pragma once
#include <stdio.h>
#include "glew.h"
#include <GL/gl.h>
#include <iostream>
#include <math.h>
#include "body.h"
#include "Eigen/Dense"
#include "options.h"
#include "math_utils.h"


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
		resolveIfContact();
		updateTails();
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
		float newSpeed = speed * multiplier;
		if (newSpeed > 9 || newSpeed < 0.062) { return; }
		speed = newSpeed;
	}

	void changeSize(float rad_change)
	{
		for (int i = 0; i < 3; i++) {
			if (bList[i]->selected == true)
			{
				bList[i]->changeSize(rad_change);
			}
		}
	}

	// Psuedo circular buffer.
	void changeSelected(int change)
	{
		bufferChangeCount+= change;
		int sel = bufferChangeCount % (int(Selections::MAX_NUM_SELECTIONS) + 1);
		if (sel < 0) { sel = sel * -1; }
		for (int i = 0; i < 3; i++)
		{
			if (bList[i]->id == sel || sel == int(Selections::ALL)) { bList[i]->selected = true; }
			else { bList[i]->selected = false; }
		}
	}

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

		b1.prevPos = b1.S.pos;
		b1.S.pos = b1.S.pos + (b1.S.vel * dt * speed);
		b2.prevPos = b2.S.pos;
		b2.S.pos = b2.S.pos + (b2.S.vel * dt * speed);
		b3.prevPos = b3.S.pos;
		b3.S.pos = b3.S.pos + (b3.S.vel * dt * speed);
	}

	void resolveIfContact()
	{
		if (inContact(b1, b2) == true) { 
			resolveContact(b1, b2); 
		}
		if (inContact(b1, b3) == true) { 
			resolveContact(b1, b3); 
		}
		if (inContact(b2, b3) == true) { 
			resolveContact(b2 , b3); 
		}
	}

	bool inContact(Body& bodyA, Body& bodyB) 
	{
		if (findDist(bodyA.S.pos, bodyB.S.pos) <= (bodyA.r + bodyB.r)){
			return true;
		}
		else {return false;}
	}

	void resolveContact(Body& bodyA, Body& bodyB) 
	{
		float dtAfter = moveToPtOfContact(bodyA,bodyB);
		resolveTranslationalCollision(bodyA, bodyB);
		// find the change in vel from rotation and add that vector to final vel. 
		// What direction will that be? Do the cross product of the rotational axes??
		//resolveRotationalCollision(bodyA, bodyB);

		// I think this is leaving the position still inside sometimes??? Something probably going on here. 
		bodyA.S.pos = bodyA.S.pos + (bodyA.S.vel * dtAfter * speed);
		bodyB.S.pos = bodyB.S.pos + (bodyB.S.vel * dtAfter * speed);
	}

	// Elastic collision with a coefficient of restitution. 
	void resolveTranslationalCollision(Body& bodyA, Body& bodyB)
	{
		Eigen::Vector3f ContactVecA = bodyB.S.pos - bodyA.S.pos;
		Eigen::Vector3f ContactRotAxisA = findCrossProduct(bodyA.S.vel, ContactVecA);

		Eigen::Vector3f ContactVecB = bodyA.S.pos - bodyB.S.pos;
		Eigen::Vector3f ContactRotAxisB = findCrossProduct(bodyB.S.vel, ContactVecB);

		// If one of the velocity vectors has a zero magnitude, this will not work.
		double contactVecVelAngA = findAngDotProductR(ContactVecA, bodyA.S.vel);
		double contactVecVelAngB = findAngDotProductR(ContactVecB, bodyB.S.vel);

		Eigen::Vector3f componentVelBefContactA = findUnit(ContactVecA) * (findMagVec(bodyA.S.vel) * cos(contactVecVelAngA));
		Eigen::Vector3f componentVelOrthoContactA = bodyA.S.vel - componentVelBefContactA;
		Eigen::Vector3f componentVelBefContactB = findUnit(ContactVecB) * (findMagVec(bodyB.S.vel) * cos(contactVecVelAngB));
		Eigen::Vector3f componentVelOrthoContactB = bodyB.S.vel - componentVelBefContactB;


		//TODO left off here. 
		// Collapse the collision to 1d on the contact axis. Need to define which way is positive in 1d.
		// Define bodyA ContactVecA as the positive direction. 
		// For A, if the angle is acute, the velocity on the 1d ContactVecA axis is positve. 
		double velA_bef = findMagVec(componentVelBefContactA) * angleAcuteR(contactVecVelAngA);
		// For A if the angle is acute, it is moving in the negative direction on ContactVecA.
		double velB_bef = findMagVec(componentVelBefContactB) * (-1) * angleAcuteR(contactVecVelAngB);
		double velA_aft = solveElasticCollision1D(bodyA.m, bodyB.m, velA_bef, velB_bef,coefRest);
		double velB_aft = solveElasticCollision1D(bodyB.m, bodyA.m, velB_bef, velA_bef, coefRest);
	
		Eigen::Vector3f componentVelAftContactA = velA_aft * findUnit(ContactVecA);
		Eigen::Vector3f componentVelAftContactB = velB_aft * findUnit(ContactVecA);

		bodyA.S.vel = componentVelAftContactA + componentVelOrthoContactA;
		bodyB.S.vel = componentVelAftContactB + componentVelOrthoContactB;
	}


	void resolveRotationalCollision(Body& bodyA, Body& bodyB,Eigen::Vector3f& VelA, Eigen::Vector3f& VelB)
	{
		// Inelastic collision. Finds the axis of rotation after the collision, and spin velocity. 
		Eigen::Vector3f rotCollisionVelModA, rotCollisionVelModB;

		// Remember to add the rotation component to cartesian velocity.
		bodyA.S.vel += rotCollisionVelModA;
		bodyB.S.vel += rotCollisionVelModB;	
	}

	void updateTails()
	{
		b1.updateTail();
		b2.updateTail();
		b3.updateTail();
	}

	float moveToPtOfContact(Body& bodyA, Body& bodyB)
	{
		// Binary search to find the point in space where contact was made for each body.
		// Returns the dt left to simulate after the collision.
		Eigen::Vector3f segVecA = bodyA.S.pos - bodyA.prevPos;
		Eigen::Vector3f segVecB = bodyB.S.pos - bodyB.prevPos;
		Eigen::Vector3f ptA, ptB;
		float totalRad = bodyA.r + bodyB.r;
		float leftX = 0;
		float rightX = 1;
		float X;
		float interferance = (totalRad - findDist(bodyA.S.pos, bodyB.S.pos));
		int maxIters = 100;
		int i; 
		for (i = 0; i < maxIters; i++)
		{
			X = (rightX + leftX) / 2;
			ptA = bodyA.prevPos + (X * segVecA);
			ptB = bodyB.prevPos + (X * segVecB);
			interferance = (totalRad - findDist(ptA, ptB));
			if (interferance > 0) {
				rightX = X;
			}
			else { leftX = X; }
			// Make sure the bodies don't interfere. 
			if (interferance <= 0.0f && interferance > -0.001f) { break; }
		}
		// It could be the case that the binary search fails because the bodies collided too fast and are now moving
		// away from each other. If so, set pos to prevPos, which should be not in contact. 
		if (i == maxIters - 1) {
			ptA = bodyA.prevPos;
			ptB = bodyB.prevPos;
			X = 0;
		}
		bodyA.S.pos = ptA;
		bodyB.S.pos = ptB;
		return dt - (X * dt);
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

	double prevTime, dt;
	Eigen::Vector3f c;
	Body b1, b2, b3;
	Body* bList[3] = {&b1,&b2,&b3};
	Eigen::Vector3f center;
	float speed;
	float coefRest = 0.9f;
	int bufferChangeCount;

};