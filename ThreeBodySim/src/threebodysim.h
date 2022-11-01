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
	ThreeBodySim(Body& body1, Body& body2, Body& body3, Body& bound)
	{
		b1 = body1;
		b2 = body2;
		b3 = body3;
		boundary = bound;
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
		changeSelected(int(Selections::NONE));
	}

	void step()
	{
		findAccels();
		updateTime();
		updateBodies();
		resolveIfContact();
		updateCenter();
		updateTails();
	}

	void drawBodies(Views view,Tails tailOption)
	{
		Eigen::Vector3f translation;
		translation = boundary.S.pos;
		glEnable(GL_NORMALIZE);

		glShadeModel(GL_SMOOTH);
		//SetMaterial(0, 1, 1, 0.8f);
		b1.drawObliq(translation);

		glShadeModel(GL_SMOOTH);
		//SetMaterial(0, 1, 1, 0.8f);
		b2.drawObliq(translation);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, b3.texture);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glShadeModel(GL_FLAT);
		//SetMaterial(0, 1, 1, 0.8f);
		b3.drawObliq(translation);
		glDisable(GL_TEXTURE_2D);
	}

	void initLists()
	{
		b1.initLists();
		b2.initLists();
		b3.initLists();
		boundary.initLists();
	}

	void initTextures()
	{
		unsigned char* TextureArray1 = BmpToTexture(getFullPath("\\textures\\worldtex.bmp"), &b1.texW, &b1.texH);
		unsigned char* TextureArray2 = BmpToTexture(getFullPath("\\textures\\worldtex.bmp"), &b1.texW, &b1.texH);
		//unsigned char* TextureArray = BmpToTexture((char *)"C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\textures\\worldtex.bmp", &texW, &texH);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &b1.texture); // assign binding �handles�
		glGenTextures(1, &b2.texture); // assign binding �handles�

		glBindTexture(GL_TEXTURE_2D, b1.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, b1.texW, b1.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray1);

		glBindTexture(GL_TEXTURE_2D, b2.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, b2.texW, b2.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray1);


		glBindTexture(GL_TEXTURE_2D, b1.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, b1.texW, b1.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray1);

		glBindTexture(GL_TEXTURE_2D, b2.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, b2.texW, b2.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray1);
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
			if (int(bList[i]->bType) == sel || sel == int(Selections::ALL)) { bList[i]->selected = true; }
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
		//b2.S.vel = b2.S.vel + (b2.S.acc * dt * speed);
		//b3.S.vel = b3.S.vel + (b3.S.acc * dt * speed);

		boundary.S.pos = Eigen::Vector3f{ 0,0,0 };
		timeSinceBoundContact += dt;
		b1.prevPos = b1.S.pos;
		b1.S.pos = b1.S.pos + (b1.S.vel * dt * speed);
		//b2.prevPos = b2.S.pos;
		//b2.S.pos = b2.S.pos + (b2.S.vel * dt * speed);
		//b3.prevPos = b3.S.pos;
		//b3.S.pos = b3.S.pos + (b3.S.vel * dt * speed);
	}

	void updateTails()
	{
		b1.updateTail();
		b2.updateTail();
		b3.updateTail();
	}

	void updateCenter()
	{
		center = (b1.S.pos + b2.S.pos + b3.S.pos) / 3;
	}


	// ##################### RESOLVING CONTACT FUNCTIONS ##################### //

	void resolveIfContact()
	{
		// All combinations of boundary and bodies
		int i, j;
		for (i = 0; i < 3; i++)
		{
			if (inContact(boundary, *bList[i]) == true) { 
				resolveContact(boundary, *bList[i]);
				timeSinceBoundContact = 0.0f;
			}
			j = i + 1;
			for (j; j < 3; j++)
			{
				if (inContact(*bList[i], *bList[j]) == true) {resolveContact(*bList[i], *bList[j]);}
			}
		}
	}

	bool inContact(Body& bodyA, Body& bodyB) 
	{
		int dir = findSignf(bodyA.r * bodyB.r);
		if (dir*findDist(bodyA.S.pos, bodyB.S.pos) <= dir*abs((bodyA.r + bodyB.r))){
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
		bodyA.S.pos = bodyA.S.pos + (bodyA.S.vel * dtAfter * speed);
		bodyB.S.pos = bodyB.S.pos + (bodyB.S.vel * dtAfter * speed);
	}


	float moveToPtOfContact(Body& bodyA, Body& bodyB)
	{
		// Binary search to find the point in space where contact was made for each body.
		// Returns the dt left to move after the collision.
		Eigen::Vector3f segVecA = bodyA.S.pos - bodyA.prevPos;
		Eigen::Vector3f segVecB = bodyB.S.pos - bodyB.prevPos;
		Eigen::Vector3f ptA, ptB;
		int dir = findSignf(bodyA.r * bodyB.r);
		float totalRad = abs(bodyA.r + bodyB.r);
		float tolerance = 0.005f * (totalRad);
		float criteria1 = -1 * dir * tolerance;
		float criteria2 = 0.0f;
		float critLow = criteria1 < criteria2 ? criteria1 : criteria2;
		float critHigh = criteria1 > criteria2 ? criteria1 : criteria2;
		float leftX = 0;
		float rightX = 1;
		float X, interferance;
		int maxIters = 20;
		int i;
		for (i = 0; i < maxIters; i++)
		{
			X = (rightX + leftX) / 2;
			ptA = bodyA.prevPos + (X * segVecA);
			ptB = bodyB.prevPos + (X * segVecB);
			interferance = ((dir*totalRad) - (dir*findDist(ptA, ptB)));
			if (interferance > 0) {
				rightX = X;
			}
			else { leftX = X; }
			if (interferance <= critHigh && interferance > critLow) { break; }
		}
		// It could be the case that the binary search fails because the bodies collided too fast and are now moving
		// away from each other. If so, move away until in a good position.  
		if (i == maxIters) {
			moveUntilNoContact(bodyA, bodyB, dir);
			return 0.0f;
		}
		bodyA.S.pos = ptA;
		bodyB.S.pos = ptB;
		return dt - (X * dt);
	}

	void moveUntilNoContact(Body& bodyA, Body& bodyB, int dir)
	{
		float totalRad = abs(bodyA.r + bodyB.r);
		float stepSize = 0.5f * (totalRad);
		Eigen::Vector3f ContactVecA = dir * findUnit(bodyB.S.pos - bodyA.S.pos);
		Eigen::Vector3f ContactVecB = dir * findUnit(bodyA.S.pos - bodyB.S.pos);
		Eigen::Vector3f ptA, ptB;
		float interferance;
		do {
			ptA = bodyA.prevPos + (stepSize * -ContactVecA);
			ptB = bodyB.prevPos + (stepSize * -ContactVecB);
			interferance = dir * (totalRad - findDist(ptA, ptB));
		} while (interferance > stepSize);
		bodyA.S.pos = ptA;
		bodyB.S.pos = ptB;
	}

	// Elastic collision with a coefficient of restitution. 
	void resolveTranslationalCollision(Body& bodyA, Body& bodyB)
	{
		Eigen::Vector3f ContactVecA = bodyB.S.pos - bodyA.S.pos;
		Eigen::Vector3f ContactRotAxisA = findCrossProduct(bodyA.S.vel, ContactVecA);

		Eigen::Vector3f ContactVecB = bodyA.S.pos - bodyB.S.pos;
		Eigen::Vector3f ContactRotAxisB = findCrossProduct(bodyB.S.vel, ContactVecB);

		// TODO If one of the velocity vectors has a zero magnitude, this will not work.
		double contactVecVelAngA = findAngDotProductR(ContactVecA, bodyA.S.vel);
		double contactVecVelAngB = findAngDotProductR(ContactVecB, bodyB.S.vel);

		Eigen::Vector3f componentVelBefContactA = findUnit(ContactVecA) * (findMagVec(bodyA.S.vel) * cos(contactVecVelAngA));
		Eigen::Vector3f componentVelOrthoContactA = bodyA.S.vel - componentVelBefContactA;
		Eigen::Vector3f componentVelBefContactB = findUnit(ContactVecB) * (findMagVec(bodyB.S.vel) * cos(contactVecVelAngB));
		Eigen::Vector3f componentVelOrthoContactB = bodyB.S.vel - componentVelBefContactB;

		// Collapse the collision to 1d on the contact axis. This is possible because, for spheres, the contact axis is also in 
		// the direction of the center of mass.
		// Need to define which way is positive in 1d. Define bodyA ContactVecA as the positive direction. 
		// For A, if the angle is acute, the velocity on the 1d ContactVecA axis is positve. 
		double velA_bef = findMagVec(componentVelBefContactA) * angleAcuteR(contactVecVelAngA);
		// For B if the angle is acute, it is moving in the negative direction on ContactVecA.
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

		Eigen::Vector3f ContactVecA = bodyB.S.pos - bodyA.S.pos;
		Eigen::Vector3f ContactRotAxisA = findCrossProduct(bodyA.S.vel, ContactVecA);

		Eigen::Vector3f ContactVecB = bodyA.S.pos - bodyB.S.pos;
		Eigen::Vector3f ContactRotAxisB = findCrossProduct(bodyB.S.vel, ContactVecB);

		// Remember to add the rotation component to cartesian velocity.
		bodyA.S.vel += rotCollisionVelModA;
		bodyB.S.vel += rotCollisionVelModB;	
	}


	// ##################### UTILS ##################### //

	void updateTime()
	{
		curTime = getCurTime();
		dt = (curTime - prevTime);
		prevTime = curTime;
	}

	float getCurTime() {return ((float)glutGet(GLUT_ELAPSED_TIME)) / 1000.f;}

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
	Body b1, b2, b3,boundary;
	Body* bList[3] = {&b1,&b2,&b3};
	Eigen::Vector3f center;
	float speed, curTime;
	float coefRest = 0.5f;
	int bufferChangeCount;
	float timeSinceBoundContact = 100000;
};