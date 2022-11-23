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
	PtMass(Vector3f pos, float mass)
	{
		m_pos, m_new_pos, m_prev_pos = pos;
		m_vel, m_new_vel, m_prev_vel = Vector3f(0.0f, 0.0f, 0.0f);
		m_mass = mass;
	}

	void step_pos()
	{
		m_prev_pos = m_pos;
		m_pos = m_new_pos;
	}

	void step_vel()
	{
		m_prev_vel = m_vel;
		m_vel = m_new_vel;
	}

	// Update position and derive the velocity from that. Useful for updating the ends of the rope manually.
	void step_vel(Vector3f pos, float dt)
	{
		m_prev_pos, m_pos = pos;
		m_new_vel = (pos - m_prev_pos) * dt;
	}

	Vector3f m_pos, m_new_pos, m_prev_pos, m_vel, m_new_vel, m_prev_vel;
	float m_mass;
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
		std::vector<PtMass> pts(_num_pts, PtMass(start_pos, mass_per_pt));
		num_pts = _num_pts;
		unstretched_len = _unstretched_len;
		lo_per_seg = find_lo_per_seg(); // unstretched length per segment
		fixed_tail = _fixed_tail;
		DRAG_COEF = _drag_coef;
		GRAVITY = Vector3f(0.0f, gravity, 0.0f);
		//reset();
	};

	float mass_per_pt,k,c, unstretched_len, lo_per_seg, DRAG_COEF;
	PtMass first_pt, last_pt;
	std::vector<PtMass> pts;
	int num_pts;
	bool fixed_tail;
	Vector3f GRAVITY;

	float find_lo_per_seg()
	{
		//TODO
		return 10;
	}

	void reset()
	{
		//if (save_data == true) { reset_sim_data(); }
		initialize();
	}

	void initialize()
	{
		pts[0] = first_pt;
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