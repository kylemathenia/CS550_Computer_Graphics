#pragma once
#include <stdio.h>
#include <vector>
#include "glew.h"
#include <GL/gl.h>
#include <iostream>
#include <math.h>
#include "body.h"
#include "Eigen/Dense"
#include "options.h"
#include "math_utils.h"

using namespace Eigen;

class PtMass
{
public:
	PtMass() {}
	PtMass(Vector3f position, float m)
	{
		pos = position;
		new_pos = position;
		prev_pos = position;
		vel = Vector3f(0.0f, 0.0f, 0.0f);
		new_vel= Vector3f(0.0f, 0.0f, 0.0f);
		prev_vel = Vector3f(0.0f, 0.0f, 0.0f);
		mass = m;
	}

	void step_pos()
	{
		prev_pos = pos;
		pos = new_pos;
	}

	void step_vel()
	{
		prev_vel = vel;
		vel = new_vel;
	}

	// Update position and derive the velocity from that. Useful for updating the ends of the rope manually.
	void step_vel(Vector3f pos, float dt)
	{
		prev_pos, pos = pos;
		new_vel = (pos - prev_pos) * dt;
	}

	Vector3f pos, new_pos, prev_pos, vel, new_vel, prev_vel;
	float mass;
};

class Rope
{
public:
	Rope(float _k, float _c, Vector3f start_pos, Vector3f end_pos, int _num_pts,
		float _rope_density, float _unstretched_len, bool _fixed_tail, float gravity, 
		float _drag_coef)
	{
		mass_per_pt = (_rope_density * _unstretched_len) / _num_pts;
		first_pt = PtMass(start_pos, mass_per_pt);
		last_pt = PtMass(end_pos, mass_per_pt);
		k = _k;
		c = _c;
		num_pts = _num_pts;
		unstretched_len = _unstretched_len;
		lo_per_seg = _unstretched_len / float(_num_pts - 1); // unstretched length per segment
		fixed_tail = _fixed_tail;
		DRAG_COEF = _drag_coef;
		GRAVITY = Vector3f(0.0f, gravity, 0.0f);
		reset();
	};

	float mass_per_pt,k,c, unstretched_len, lo_per_seg, DRAG_COEF;
	PtMass first_pt, last_pt;
	std::vector<PtMass> pts;
	int num_pts;
	bool fixed_tail;
	Vector3f GRAVITY;

	void reset()
	{
		for (int i = 0; i < num_pts; i++) { pts.push_back(PtMass(first_pt.pos, mass_per_pt)); }
		//if (save_data == true) { reset_sim_data(); }
		initialize();
	}

	void initialize()
	{
		pts[0] = first_pt;
		pts.back() = last_pt;
		Vector3f diff = last_pt.pos - first_pt.pos;
		for (int i = 1; i < num_pts; i++){
			float dec_percent = float(i) / float((num_pts - 1));
			Vector3f pos = first_pt.pos + (diff * dec_percent);
			pts[i] = PtMass(pos, mass_per_pt);
		}
	}

	//void reset_sim_data()
	//{
	//	// Delete the dynamic array and initialize an empty one. 
	//	// Could figure out the size of array at the sim() call, given number of steps...
	//}


	// ##################### UTILS ##################### //

	void updateTime()
	{
		curTime = getCurTime();
		dt = (curTime - prevTime);
		prevTime = curTime;
	}

	float getCurTime() { return ((float)glutGet(GLUT_ELAPSED_TIME)) / 1000.f; }

	Vector3f numerator(Body a, Body b)
	{
		return -9.81 * b.m * (a.S.pos - b.S.pos);
	}


	double prevTime, dt;
	Body b1, b2, b3, boundary;
	Body* bList[3] = { &b1,&b2,&b3 };
	Vector3f center;
	float curTime;
};