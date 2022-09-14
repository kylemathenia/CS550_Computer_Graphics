#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define _USE_MATH_DEFINES
#include <math.h>

unsigned char*
BmpToTexture(char* filename, int* width, int* height);

int
ReadInt(FILE* fp);

short
ReadShort(FILE* fp);

void
HsvRgb(float hsv[3], float rgb[3]);

void
Cross(float v1[3], float v2[3], float vout[3]);

float
Dot(float v1[3], float v2[3]);

float
Unit(float vin[3], float vout[3]);

struct pt2i {
	int x;
	int y;
};

struct pt2f {
	double x;
	double y;
};

struct pt3i {
	double x;
	double y;
	double z;
};

pt2i
GetDesktopResolution();

float
ElapsedSeconds();

void
DoRasterString(float x, float y, float z, char* s);


void
DoStrokeString(float x, float y, float z, float ht, char* s);