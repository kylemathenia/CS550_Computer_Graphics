#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#include <iostream>

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
	long x;
	long y;
};

struct pt2f {
	double x;
	double y;
};

struct pt3i {
	long x;
	long y;
	long z;
};

struct pt3f {
	double x;
	double y;
	double z;
};

struct BtmStruct {
	char* filename;
	int width;
	int height;
};

pt2i
GetDesktopResolution();

float
ElapsedSeconds();

void
DoRasterString(float x, float y, float z, char* s);

void
DoStrokeString(float x, float y, float z, float ht, char* s);

std::string get_current_dir();

char* getFullPath(const char* relPath);


// Assignment 4 helper functions.

float* Array3(float a, float b, float c);

float* MulArray3(float factor, float array0[3]);

void SetMaterial(float r, float g, float b, float shininess);

void SetPointLight(int ilight, float x, float y, float z, float r, float g, float b);

void SetSpotLight(int ilight, float x, float y, float z, float xdir, float ydir, float zdir, float r, float g, float b);

