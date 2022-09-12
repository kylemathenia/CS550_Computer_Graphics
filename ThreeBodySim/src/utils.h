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