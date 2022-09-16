#pragma once
#include <tgmath.h>
#include "Eigen/Dense"
#include "math_utils.h"

//const float  PI = 3.14159265;
//const float  DEG90 = 90.0f;
//const float  DEG180 = 180.0f;
//const float  DEG270 = 270.0f;
//const float  DEG360 = 360.0f;
//const float  RAD90 = 1.57079633f;
//const float  RAD180 = PI;
//const float  RAD270 = 5.3796286f;
//const float  RAD360 = 6.2831853f;

double findSphereVolume(float r)
{
	return (4.0f * 3.14 * pow(r, 3)) / 3.0f;
}

double findDist(Eigen::Vector3f p1, Eigen::Vector3f p2)
{
	return sqrt(pow(p1(0) - p2(0), 2) + pow(p1(1) - p2(1), 2) + pow(p1(2) - p2(2), 2));
}

Eigen::Vector3f findCrossProduct(Eigen::Vector3f b, Eigen::Vector3f a)
{
	// The rotation angle is the cross product.
	float i = (a(1) * b(2)) - (a(2) * b(1));
	float j = (a(0) * b(2)) - (a(2) * b(0));
	float k = (a(0) * b(1)) - (a(1) * b(0));
	return Eigen::Vector3f { i, j, k };
}

double findMagVec(Eigen::Vector3f x)
{
	return sqrt(pow(x(0), 2) + pow(x(1), 2) + pow(x(2), 2));
}

double findAngDotProductD(Eigen::Vector3f a, Eigen::Vector3f b)
{
	// returns angle in degrees from 0 to 180
	return (180.0f / PI) * findAngDotProductR(a,b);
}

double findAngDotProductR(Eigen::Vector3f a, Eigen::Vector3f b)
{
	// returns angle in radians from 0 to PI
	double numer = findDot(a, b);
	double denom = findMagVec(a) * findMagVec(b);
	double ret = acos(numer / denom);
	// Make sure it doesn't break acos.
	if (isnan(ret) == true) {
		if (findSignf(numer) > 0) {return 0.0f;}
		else { return RAD180; }
	}
	return ret;
}

double findDot(Eigen::Vector3f a, Eigen::Vector3f b)
{
	return (a(0) * b(0)) + (a(1) * b(1)) + (a(2) * b(2));
}

Eigen::Vector3f findUnit(Eigen::Vector3f vec) { return vec / findMagVec(vec); }

double solveElasticCollision1D(double m1, double m2, double v1, double v2, double e)
{
	// Notice the order of arguments matters.
	//return ((m1 * v1) + (m2 * v2) + (m2 * e * (v2 - v1))) / (m1 + m2);
	return ((2*m2*v2) + (v1*(m1-m2)))/ (m1 + m2);
}

int angleAcuteR(float ang)
{
	// expects angle in radians
	float rem = fmod(abs(ang), RAD360);
	if (RAD180 - fmod(rem, RAD180) > RAD90) { return 1;}
	else { return -1; }
}

int angleAcuteD(float ang)
{
	// expects angle in degrees
	float rem = fmod(abs(ang), DEG360);
	if (DEG180 - fmod(rem, DEG180) < DEG90) { return 1; }
	else { return -1; }
}

int findSignf(double x)
{
	return (x > 0) - (x < 0);
}