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

	double prevTime, dt;
	Body b1, b2, b3, boundary;
	Body* bList[3] = { &b1,&b2,&b3 };
	Vector3f center;
	float curTime;

	GLuint obj_list = glGenLists(1);

	void reset()
	{
		for (int i = 0; i < num_pts; i++) { pts.push_back(PtMass(first_pt.pos, mass_per_pt)); }
		//if (save_data == true) { reset_sim_data(); }
		initialize();
		init_list(false);
	}

	void initialize()
	{
		// Initialize the points linearly between first and last points.
		pts[0] = first_pt;
		pts.back() = last_pt;
		Vector3f diff = last_pt.pos - first_pt.pos;
		for (int i = 1; i < num_pts; i++){
			float dec_percent = float(i) / float((num_pts - 1));
			Vector3f pos = first_pt.pos + (diff * dec_percent);
			pts[i] = PtMass(pos, mass_per_pt);
		}
	}

	void step()
	{
		//updateTime();
		dt = 0.05;
		PtMass cur_pt, next_pt, prev_pt;
		for (int i = 1; i < num_pts-1; i++)
		{
			//cur_pt = pts[i];
			////if (&cur_pt == &first_pt || &cur_pt == &last_pt) { continue; }
			//prev_pt = pts[i - 1];
			//next_pt = pts[i + 1];
			step_verlet(pts[i], pts[i - 1], pts[i + 1], true);
		}

		if (fixed_tail == false) { step_verlet(pts.back(), pts[num_pts-2], pts[num_pts - 2], false); }
		apply_steps();
		init_list(true);
	}

	void step_verlet(PtMass& cur_pt, PtMass& prev_pt, PtMass& next_pt,bool next_pt_exists)
	{
		Vector3f acc = find_acc(cur_pt, prev_pt, next_pt,next_pt_exists);
		Vector3f p1 = (2 * cur_pt.pos);
		Vector3f p2 = (pow(dt, 2) * acc);
		Vector3f new_pos = p1 - cur_pt.prev_pos + p2;
		cur_pt.new_pos = (2 * cur_pt.pos) - cur_pt.prev_pos + ((pow(dt, 2) * acc));
		cur_pt.new_vel = (cur_pt.new_pos - cur_pt.pos) * dt;
	}

	void apply_steps()
	{
		for (int i = 1; i < num_pts - 1; i++)
		{
			pts[i].step_pos();
			pts[i].step_vel();
		}
		if (fixed_tail == false)
		{
			last_pt.step_pos();
			last_pt.step_vel();
		}
	}

	Vector3f find_acc(PtMass& cur_pt, PtMass& prev_pt, PtMass& next_pt, bool next_pt_exists)
	{
		Vector3f gravitational_force, force_next_pt, force_prev_pt, drag;
		gravitational_force = cur_pt.mass * GRAVITY;
		if (next_pt_exists == true) { force_next_pt = find_force_bet_pts(cur_pt.pos, cur_pt.vel, next_pt.pos, next_pt.vel); }
		else { force_next_pt = Vector3f(0, 0, 0); }
		force_prev_pt = find_force_bet_pts(cur_pt.pos, cur_pt.vel, prev_pt.pos, prev_pt.vel);
		drag = find_drag(cur_pt.vel);
		return (force_next_pt + force_prev_pt + drag + gravitational_force) / cur_pt.mass;
	}

	Vector3f find_force_bet_pts(Vector3f curPtPos, Vector3f curPtVel, Vector3f otherPtPos, Vector3f otherPtVel)
	{
		Vector3f unit_vec_to_pt, spring_force, visc_force;
		unit_vec_to_pt = unit_vec(otherPtPos - curPtPos);
		spring_force = find_spring_force(curPtPos, otherPtPos, unit_vec_to_pt);
		visc_force = find_visc_force(curPtVel, otherPtVel, curPtPos, otherPtPos, unit_vec_to_pt);
		return spring_force + visc_force;
	}

	Vector3f find_spring_force(Vector3f curPtPos, Vector3f otherPtPos, Vector3f unit_vec_to_pt)
	{
		double dist, deflection;
		dist = find_dist(curPtPos, otherPtPos);
		deflection = dist - lo_per_seg;
		if (deflection < 0) { return Vector3f(0, 0, 0); }
		else { return k * deflection * unit_vec_to_pt; }
	}

	Vector3f find_visc_force(Vector3f curPtVel, Vector3f otherPtVel, Vector3f curPtPos, Vector3f otherPtPos, Vector3f unit_vec_to_pt)
	{
		Vector3f relative_vel = curPtVel - otherPtVel;
		double relative_speed = relative_vel.dot(unit_vec_to_pt);
		Vector3f component_vel = relative_speed*unit_vec_to_pt;
		double dist = find_dist(otherPtPos, curPtPos);
		double deflection = dist - lo_per_seg;
		if (deflection < 0){return Vector3f(0, 0, 0);}
		else { return -component_vel * c; }
	}

	Vector3f find_drag(Vector3f vel)
	{
		return 0.5 * pow(vel.norm(), 2) * DRAG_COEF * unit_vec(vel);
	}

	void init_list(bool updating)
	{
		if (updating == true) { glDeleteLists(obj_list, 1); }
		glNewList(obj_list, GL_COMPILE);
		glLineWidth((GLfloat)2);
		glBegin(GL_LINES);
		for (int i = 0; i < num_pts; i++)
		{
			glVertex3f(pts[i].pos[0], pts[i].pos[1], pts[i].pos[2]);
		}
		glEnd();
		glEndList();
	}

	void draw()
	{
		glPushMatrix();
		glEnable(GL_DEPTH_TEST);
		//glColor3f(Colors[c][0], Colors[c][1], Colors[c][2]);
		//glTranslatef(d(0), d(1), d(2));
		//glRotatef(ang, rotAxis(0), rotAxis(1), rotAxis(2));
		//glScalef(scale(0), scale(1), scale(2));
		GLuint list = glGenLists(1);
		glNewList(list, GL_COMPILE);
		glLineWidth((GLfloat)2);
		glBegin(GL_LINES);
		for (int i = 0; i < num_pts; i++)
		{
			glVertex3f(pts[i].pos[0], pts[i].pos[1], pts[i].pos[2]);
		}
		glEnd();
		glEndList();

		glCallList(list);
		glPopMatrix();
	}

	//void reset_sim_data()
	//{
	//	// Delete the dynamic array and initialize an empty one. 
	//	// Could figure out the size of array at the sim() call, given number of steps...
	//}


	// ##################### UTILS ##################### //


	Vector3f unit_vec(Vector3f vec)
	{
		if (vec.isZero(0) == true) { return Vector3f(0, 0, 0); }
		double mag = double(vec.norm());
		return vec / mag;
	}

	double find_dist(Vector3f pt1, Vector3f pt2)
	{
		double difx = pt2[0] - pt1[0];
		double dify = pt2[1] - pt1[1];
		double difz = pt2[2] - pt1[2];
		double tot = pow(difx, 2) + pow(dify, 2) + pow(difz, 2);
		return pow(tot, 0.5);
	}

	void updateTime()
	{
		curTime = getCurTime();
		dt = (curTime - prevTime);
		prevTime = curTime;
	}

	float getCurTime() { return ((float)glutGet(GLUT_ELAPSED_TIME)) / 1000.f; }

};