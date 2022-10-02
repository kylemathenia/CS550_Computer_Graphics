// standard
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <chrono>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <tgmath.h>
#include <direct.h>
#define GetCurrentDir _getcwd

#include <chrono>
#include <thread>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

// dependencies
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "Eigen/Dense"

// Some custom math functions I'm not going to paste here...
#include "math_utils.h"




// src
/////////////////////// INCLUDES PASTED HERE /////////////////////// 

///////////////////////////
//#include "utils.h"
///////////////////////////
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



// read a BMP file into a Texture:
#define VERBOSE		false
#define BMP_MAGIC_NUMBER	0x4d42
#ifndef BI_RGB
#define BI_RGB			0
#define BI_RLE8			1
#define BI_RLE4			2
#endif


// bmp file header:
struct bmfh
{
	short bfType;		// BMP_MAGIC_NUMBER = "BM"
	int bfSize;		// size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes;		// # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		// info header size, should be 40
	int biWidth;		// image width
	int biHeight;		// image height
	short biPlanes;		// #color planes, should be 1
	short biBitCount;	// #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression;	// BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed;		// # colors in the palette
	int biClrImportant;
} InfoHeader;

// read a BMP file into a Texture:
unsigned char*
BmpToTexture(char* filename, int* width, int* height)
{
	FILE* fp;
#ifdef _WIN32
	errno_t err = fopen_s(&fp, filename, "rb");
	if (err != 0)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}
#else
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}
#endif

	FileHeader.bfType = ReadShort(fp);

	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if (VERBOSE) fprintf(stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
		FileHeader.bfType, FileHeader.bfType & 0xff, (FileHeader.bfType >> 8) & 0xff);
	if (FileHeader.bfType != BMP_MAGIC_NUMBER)
	{
		fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
		fclose(fp);
		return NULL;
	}

	FileHeader.bfSize = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize);

	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);

	FileHeader.bfOffBytes = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "FileHeader.bfOffBytes = %d\n", FileHeader.bfOffBytes);

	InfoHeader.biSize = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biSize = %d\n", InfoHeader.biSize);
	InfoHeader.biWidth = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biWidth = %d\n", InfoHeader.biWidth);
	InfoHeader.biHeight = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biHeight = %d\n", InfoHeader.biHeight);

	const int nums = InfoHeader.biWidth;
	const int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biPlanes = %d\n", InfoHeader.biPlanes);

	InfoHeader.biBitCount = ReadShort(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount);

	InfoHeader.biCompression = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression);

	InfoHeader.biSizeImage = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage);

	InfoHeader.biXPixelsPerMeter = ReadInt(fp);
	InfoHeader.biYPixelsPerMeter = ReadInt(fp);

	InfoHeader.biClrUsed = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed);

	InfoHeader.biClrImportant = ReadInt(fp);

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	// pixels will be stored bottom-to-top, left-to-right:
	unsigned char* texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\n");
		return NULL;
	}

	// extra padding bytes:
	int requiredRowSizeInBytes = 4 * ((InfoHeader.biBitCount * InfoHeader.biWidth + 31) / 32);
	if (VERBOSE)	fprintf(stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes);

	int myRowSizeInBytes = (InfoHeader.biBitCount * InfoHeader.biWidth + 7) / 8;
	if (VERBOSE)	fprintf(stderr, "myRowSizeInBytes = %d\n", myRowSizeInBytes);

	int oldNumExtra = 4 * (((3 * InfoHeader.biWidth) + 3) / 4) - 3 * InfoHeader.biWidth;
	if (VERBOSE)	fprintf(stderr, "Old NumExtra padding = %d\n", oldNumExtra);

	int numExtra = requiredRowSizeInBytes - myRowSizeInBytes;
	if (VERBOSE)	fprintf(stderr, "New NumExtra padding = %d\n", numExtra);

	// this function does not support compression:
	if (InfoHeader.biCompression != 0)
	{
		fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}

	// we can handle 24 bits of direct color:
	if (InfoHeader.biBitCount == 24)
	{
		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);
		int t;
		unsigned char* tp;
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (int s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp);		// b
				*(tp + 1) = fgetc(fp);		// g
				*(tp + 0) = fgetc(fp);		// r
			}

			for (int e = 0; e < numExtra; e++)
			{
				fgetc(fp);
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if (InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256)
	{
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32* colorTable = new struct rgba32[InfoHeader.biClrUsed];

		rewind(fp);
		fseek(fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET);
		for (int c = 0; c < InfoHeader.biClrUsed; c++)
		{
			colorTable[c].r = fgetc(fp);
			colorTable[c].g = fgetc(fp);
			colorTable[c].b = fgetc(fp);
			colorTable[c].a = fgetc(fp);
			if (VERBOSE)	fprintf(stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
				c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a);
		}

		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);
		int t;
		unsigned char* tp;
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (int s = 0; s < nums; s++, tp += 3)
			{
				int index = fgetc(fp);
				*(tp + 0) = colorTable[index].r;	// r
				*(tp + 1) = colorTable[index].g;	// g
				*(tp + 2) = colorTable[index].b;	// b
			}

			for (int e = 0; e < numExtra; e++)
			{
				fgetc(fp);
			}
		}

		delete[] colorTable;
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt(FILE* fp)
{
	const unsigned char b0 = fgetc(fp);
	const unsigned char b1 = fgetc(fp);
	const unsigned char b2 = fgetc(fp);
	const unsigned char b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

short
ReadShort(FILE* fp)
{
	const unsigned char b0 = fgetc(fp);
	const unsigned char b1 = fgetc(fp);
	return (b1 << 8) | b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb 
void
HsvRgb(float hsv[3], float rgb[3])
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while (h >= 6.)	h -= 6.;
	while (h < 0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:
	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	float i = (float)floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r = 0., g = 0., b = 0.;			// red, green, blue
	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

pt2i
GetDesktopResolution()
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	return pt2i{ desktop.right ,desktop.bottom };
}

// return the number of seconds since the start of the program:
float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:
	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:
	return (float)ms / 1000.f;
}

// use glut to display a string of characters using a raster font:
void
DoRasterString(float x, float y, float z, char* s)
{
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}

// use glut to display a string of characters using a stroke font:
void
DoStrokeString(float x, float y, float z, float ht, char* s)
{
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}


std::string get_current_dir() {
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
}

char* getFullPath(const char* relPath)
{
	std::string cwd = get_current_dir();
	char* path = new char[cwd.length() + 1];
	strcpy(path, cwd.c_str());
	strcat(path, relPath);
	return path;
}



///////////////////////////
//#include "color.h"
///////////////////////////

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char* ColorNames[] =
{
	(char*)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order
const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};



///////////////////////////
//#include "options.h"
///////////////////////////
enum Projections
{
	ORTHO,
	PERSP
};

enum ButtonVals
{
	RESET,
	SOFT_RESET,
	QUIT
};

enum Views
{
	CENTER,
	BODY1,
	BODY2,
	BODY3,
	MAX_NUM_VIEWS = BODY3
};

enum class Selections
{
	B1,
	B2,
	B3,
	ALL,
	NONE,
	MAX_NUM_SELECTIONS = NONE
};

enum class Bodies
{
	B1,
	B2,
	B3,
	BOUNDARY
};

enum Tails
{
	LINES,
	CYLINDERS,
	SPHERES,
	NONE
};

enum EventEnums
{
	ESCAPE = 0x1b,
	SCROLL_WHEEL_UP = 3,
	SCROLL_WHEEL_DOWN = 4,
	SCROLL_WHEEL_CLICK_FACTOR = 5,
	LEFT = 4,
	MIDDLE = 2,
	RIGHT = 1
};



///////////////////////////
//#include "Shapes/shapes.h"
///////////////////////////
GLuint getCubeList(float BOXSIZE);
GLuint getSphereList(float radius, int slices, int stacks);
GLuint getDistortedSphereList(float radius, int slices, int stacks, float curTime);
GLuint getAxesList(const GLfloat axesWidth);
GLuint getConeList(float radBot, float radTop, float height, int slices, int stacks, bool top, bool bot);

struct point
{
	float x, y, z;		// coordinates
	float nx, ny, nz;	// surface normal
	float s, t;		// texture coords
};

inline
void
DrawPoint(struct point* p)
{
	glNormal3fv(&p->nx);
	glTexCoord2fv(&p->s);
	glVertex3fv(&p->x);
}

int		NumLngs, NumLats;
struct point* Pts;

inline
struct point*
	PtsPointer(int lat, int lng)
{
	if (lat < 0)	lat += (NumLats - 1);
	if (lng < 0)	lng += (NumLngs - 0);
	if (lat > NumLats - 1)	lat -= (NumLats - 1);
	if (lng > NumLngs - 1)	lng -= (NumLngs - 0);
	return &Pts[NumLngs * lat + lng];
}

void
OsuSphere(float radius, int slices, int stacks)
{
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if (NumLngs < 3)
		NumLngs = 3;
	if (NumLats < 3)
		NumLats = 3;

	// allocate the point data structure:
	Pts = new struct point[NumLngs * NumLats];

	// fill the Pts structure:
	for (int ilat = 0; ilat < NumLats; ilat++)
	{
		float lat = -M_PI / 2. + M_PI * (float)ilat / (float)(NumLats - 1);	// ilat=0/lat=0. is the south pole
											// ilat=NumLats-1, lat=+M_PI/2. is the north pole
		float xz = cosf(lat);
		float  y = sinf(lat);
		for (int ilng = 0; ilng < NumLngs; ilng++)				// ilng=0, lng=-M_PI and
											// ilng=NumLngs-1, lng=+M_PI are the same meridian
		{
			float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
			float x = xz * cosf(lng);
			float z = -xz * sinf(lng);
			struct point* p = PtsPointer(ilat, ilng);
			p->x = radius * x;
			p->y = radius * y;
			p->z = radius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = (lng + M_PI) / (2. * M_PI);
			p->t = (lat + M_PI / 2.) / M_PI;
		}
	}

	struct point top, bot;		// top, bottom points
	top.x = 0.;		top.y = radius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s = 0.;		top.t = 1.;

	bot.x = 0.;		bot.y = -radius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s = 0.;		bot.t = 0.;

	// connect the north pole to the latitude NumLats-2:
	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = 0; ilng < NumLngs; ilng++)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
		top.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&top);
		struct point* p = PtsPointer(NumLats - 2, ilng);	// ilat=NumLats-1 is the north pole
		DrawPoint(p);
	}
	glEnd();

	// connect the south pole to the latitude 1:
	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = NumLngs - 1; ilng >= 0; ilng--)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
		bot.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&bot);
		struct point* p = PtsPointer(1, ilng);					// ilat=0 is the south pole
		DrawPoint(p);
	}
	glEnd();

	// connect the horizontal strips:
	for (int ilat = 2; ilat < NumLats - 1; ilat++)
	{
		struct point* p;
		glBegin(GL_TRIANGLE_STRIP);
		for (int ilng = 0; ilng < NumLngs; ilng++)
		{
			p = PtsPointer(ilat, ilng);
			DrawPoint(p);
			p = PtsPointer(ilat - 1, ilng);
			DrawPoint(p);
		}
		glEnd();
	}

	// clean-up:
	delete[] Pts;
	Pts = NULL;
}




float findDistFromEq(float latitude)
{
	if (latitude <= M_PI / 2) { return (M_PI / 2) - latitude; }
	else { return latitude / 2; }
}

void
distortedOsuSphere(float radius, int slices, int stacks, float curTime)
{
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if (NumLngs < 3)
		NumLngs = 3;
	if (NumLats < 3)
		NumLats = 3;

	// allocate the point data structure:
	Pts = new struct point[NumLngs * NumLats];

	// fill the Pts structure:
	for (int ilat = 0; ilat < NumLats; ilat++)
	{
		float lat = -M_PI / 2. + M_PI * (float)ilat / (float)(NumLats - 1);	// ilat=0/lat=0. is the south pole
											// ilat=NumLats-1, lat=+M_PI/2. is the north pole
		float distFromEq = findDistFromEq(lat);
		// Need the scaling factor to go from 1 at all points to normal. 
		//https://www.desmos.com/calculator/yp8ibp3i91

		float timeMod = -1.5 * cos(curTime / 2) + 2.5;
		float scalingFactor = -1 / (cos(((distFromEq * 2) - 0.01) / timeMod));
		float xz = cosf(lat);
		float  y = sinf(lat);
		for (int ilng = 0; ilng < NumLngs; ilng++)				// ilng=0, lng=-M_PI and
											// ilng=NumLngs-1, lng=+M_PI are the same meridian
		{
			float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
			float x = xz * cosf(lng);
			float z = -xz * sinf(lng);
			struct point* p = PtsPointer(ilat, ilng);
			p->x = radius * x;
			p->y = radius * y;
			p->z = radius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			// Have a percentage from the equator. 100% at the north/south pole. Lats go from 0 to pi/2, so the equator is at pi/4.
			// Have the distortion magnitude cycle from 0 to 1 with a sinusoid. At zero, it is undistorted.
			// Have the distortion magnitude based on the % from the equator. At 0% the magnitude is 1. At the poles it is 4. 
			// The scaling function could be hyperbolic. 
			// y = 1/cos(x), where x goes from 0 to pi/2. At zero it is 1. So I need to make x = (dist from equator * 2) + 0.01
			float distFromEq = 0;

			p->s = scalingFactor * (lng + M_PI) / (2. * M_PI);
			p->t = (lat + M_PI / 2.) / M_PI;
		}
	}

	struct point top, bot;		// top, bottom points
	top.x = 0.;		top.y = radius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s = 0.;		top.t = 1.;

	bot.x = 0.;		bot.y = -radius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s = 0.;		bot.t = 0.;

	// connect the north pole to the latitude NumLats-2:
	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = 0; ilng < NumLngs; ilng++)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
		top.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&top);
		struct point* p = PtsPointer(NumLats - 2, ilng);	// ilat=NumLats-1 is the north pole
		DrawPoint(p);
	}
	glEnd();

	// connect the south pole to the latitude 1:
	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = NumLngs - 1; ilng >= 0; ilng--)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
		bot.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&bot);
		struct point* p = PtsPointer(1, ilng);					// ilat=0 is the south pole
		DrawPoint(p);
	}
	glEnd();

	// connect the horizontal strips:
	for (int ilat = 2; ilat < NumLats - 1; ilat++)
	{
		struct point* p;
		glBegin(GL_TRIANGLE_STRIP);
		for (int ilng = 0; ilng < NumLngs; ilng++)
		{
			p = PtsPointer(ilat, ilng);
			DrawPoint(p);
			p = PtsPointer(ilat - 1, ilng);
			DrawPoint(p);
		}
		glEnd();
	}

	// clean-up:
	delete[] Pts;
	Pts = NULL;
}




GLuint
getSphereList(float radius, int slices, int stacks)
{
	GLuint obj_list = glGenLists(1);
	glNewList(obj_list, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	OsuSphere(radius, slices, stacks);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	return obj_list;
}

GLuint
getDistortedSphereList(float radius, int slices, int stacks, float curTime)
{
	GLuint obj_list = glGenLists(1);
	glNewList(obj_list, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	distortedOsuSphere(radius, slices, stacks, curTime);
	glDisable(GL_TEXTURE_2D);
	glEndList();
	return obj_list;
}




///////////////////////////
//#include "body.h"
///////////////////////////

struct state {
	Eigen::Vector3f pos;
	Eigen::Vector3f vel;
	Eigen::Vector3f acc;
};

struct tailPt {
	Eigen::Vector3f pos;
	GLuint list;
};

class Body
{
public:
	Body()
	{

	};

	Body(Bodies bID, float rad, float mass, int tailLen, state init_state, enum Colors bc, enum Colors tc, int fps)
	{
		bType = bID;
		r_i = rad;
		m_i = mass;
		tailLen_i = tailLen;
		tail = new Eigen::Vector3f[tailLen];
		S_i = init_state;
		bcolor = bc;
		tcolor = tc;
		selected = false;
		// This just happens to often be a good spacing for the sphere tail most of the time. 
		tailSpacing = fps / 5;
		tailUpdateCount = -tailSpacing;
		lineVec = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
		hardReset();
	};

	void hardReset()
	{
		static int resets = 0;
		r = r_i;
		m = m_i;
		tailLen = tailLen_i;
		S = state{ S_i.pos,S_i.vel,S_i.acc };
		V = findSphereVolume(r);
		if (resets > 0) { delLists(); }
		initLists();
		initTail();
		resets++;
	}

	void initTail()
	{
		for (int i = 0; i < tailLen; i++) {
			tail[i] = S.pos;
		}
	}

	void changeSize(float rad_change)
	{
		float newR = r * rad_change;
		if (newR < bodyMinRad) { return; }
		float prev_V = V;
		r = newR;
		V = findSphereVolume(r);
		float change_ratio = V / prev_V;
		m = m * change_ratio;
		delLists();
		initLists();
	}

	void initLists()
	{
		sphereList = getSphereList(abs(r), 40, 40);
		distortedSphereList = getDistortedSphereList(abs(r), 40, 40, (float)glutGet(GLUT_ELAPSED_TIME) / 1000.f);
		cylinderList = getConeList(1, 1, 1, 15, 15, false, false);
		lineList = getLineList(1.5f);
	}

	void initTexture()
	{
		//unsigned char* TextureArray = BmpToTexture(bitmap.filename, &bitmap.width, &bitmap.height);
		//unsigned char* TextureArray = BmpToTexture((char*)"C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\textures\\worldtex.bmp", &texW, &texH);
		unsigned char* TextureArray = BmpToTexture(textPath, &texW, &texH);


		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &texture); // assign binding “handles”
		glBindTexture(GL_TEXTURE_2D, texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray);
	}

	void delLists()
	{
		glDeleteLists(sphereList, 1);
		glDeleteLists(cylinderList, 1);
		glDeleteLists(lineList, 1);
	}


	void updateTail()
	{
		// Shift the array and add one. 
		Eigen::Vector3f temp;
		Eigen::Vector3f prev_pos = tail[0];
		for (int i = 1; i < tailLen_i; i++) {
			temp = tail[i];
			tail[i] = prev_pos;
			prev_pos = temp;
		}
		tail[0] = S.pos;
		tailSpacingOffset++;
		if (tailSpacingOffset % tailSpacing == 0) { tailSpacingOffset = 0; }
		tailUpdateCount++;
	}

	GLuint getLineList(float width)
	{
		GLuint obj_list = glGenLists(1);
		glNewList(obj_list, GL_COMPILE);
		glLineWidth((GLfloat)width);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(lineVec(0), lineVec(1), lineVec(2));
		glEnd();
		glEndList();
		return obj_list;
	}


	// ##################### RENDER FUNCTIONS ##################### //

	void drawObliq(Eigen::Vector3f translation, bool seeTexture)
	{
		if (seeTexture == true) {
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}

		Eigen::Vector3f delta = S.pos - translation;
		Eigen::Vector3f scale = { 1, 1, 1 };
		Eigen::Vector3f rotAxis = { 1, 0, 0 };
		float ang = 0;
		if (distortion == true) {
			drawGlSeqOpaq(distortedSphereList, scale, delta, rotAxis, ang, bcolor);
		}
		else {
			drawGlSeqOpaq(sphereList, scale, delta, rotAxis, ang, bcolor);
		}
	}

	void drawTran(Eigen::Vector3f translation, Tails tailOption)
	{
		if (selected == true) { drawSelector(translation); }
		if (tailOption == Tails::LINES) { drawLineTail(translation, 1.0f, 1.5f); }
		else if (tailOption == Tails::CYLINDERS) { drawCylinderTail(translation, 1.0f, 0.1f); }
		else if (tailOption == Tails::SPHERES) { drawSphereTail(translation, 0.3f, 0.5f, false); }
	}

	void drawSelector(Eigen::Vector3f translation)
	{
		Eigen::Vector3f delta = S.pos - translation;
		Eigen::Vector3f scale = { selectorScale, selectorScale, selectorScale };
		Eigen::Vector3f rotAxis = { 1, 0, 0 };
		float ang = 0;
		drawGlSeqTran(sphereList, scale, delta, rotAxis, ang, 0.3f, Colors::WHITE);
	}

	void drawBoundary(Eigen::Vector3f translation, float timeSinceContact)
	{
		float visDuration = 0.5;
		float minAlpha = 0.02;
		float maxAlpha = 0.15;
		float alpha = (((maxAlpha - minAlpha) * (visDuration - timeSinceContact)) / visDuration) + minAlpha;
		if (timeSinceContact > visDuration) { alpha = minAlpha; }
		Eigen::Vector3f delta = S.pos - translation;
		Eigen::Vector3f scale = { 1, 1, 1 };
		Eigen::Vector3f rotAxis = { 1, 0, 0 };
		float ang = 0;
		drawGlSeqTran(sphereList, scale, delta, rotAxis, ang, alpha, Colors::WHITE);
	}

	void drawLineTail(Eigen::Vector3f translation, float maxAlpha, float width)
	{
		// Be able to change the width of the line.
		glDeleteLists(lineList, 1);
		lineList = getLineList(width);
		float alpha, dist, ang;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis, delta, scale;
		for (int i = 1; i < tailLen_i; i++) {
			alpha = ((float)(tailLen - i) / (float)tailLen) * maxAlpha;
			curPt = tail[i];
			nextPt = tail[i - 1];
			dist = findDist(tail[i], tail[i - 1]);
			dif = nextPt - curPt;
			rotAxis = findCrossProduct(lineVec, dif);
			ang = -findAngDotProductD(dif, lineVec);
			delta = tail[i] - translation;
			scale = { 1,dist,1 };
			drawGlSeqTran(lineList, scale, delta, rotAxis, ang, alpha, bcolor);
		}
	}

	void drawCylinderTail(Eigen::Vector3f translation, float maxAlpha, float maxScale)
	{
		//Tail using tranformations with cylinders, getting smaller and fading with length. 
		float alpha, dist, ang, scaleMod, percentComplete;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis, delta, scale;
		for (int i = 1; i < tailLen_i; i++) {
			percentComplete = ((float)(tailLen - i) / (float)tailLen);
			alpha = percentComplete * maxAlpha;
			scaleMod = percentComplete * maxScale;
			curPt = tail[i];
			nextPt = tail[i - 1];
			dist = findDist(tail[i], tail[i - 1]);
			dif = nextPt - curPt;
			rotAxis = findCrossProduct(lineVec, dif);
			ang = -findAngDotProductD(dif, lineVec);
			delta = tail[i] - translation;
			scale = { r * scaleMod,dist * 1.01f,r * scaleMod };
			drawGlSeqTran(cylinderList, scale, delta, rotAxis, ang, alpha, bcolor);
		}
	}

	void drawSphereTail(Eigen::Vector3f translation, float maxAlpha, float maxScale, bool gettingSmaller)
	{
		// Sphere tails, getting smaller and more transparent.
		float alpha, uniformScale, percentComplete;
		Eigen::Vector3f curPt, nextPt, dif, rotAxis, delta, scale;
		int j;
		for (int i = 0; i < tailLen_i - tailSpacing; i += tailSpacing) {
			percentComplete = ((float)(tailLen - (i + tailSpacingOffset))) / (tailLen);
			alpha = percentComplete * maxAlpha;
			if (gettingSmaller == true) { uniformScale = percentComplete * maxScale; }
			else { uniformScale = maxScale; }
			scale = { uniformScale ,uniformScale ,uniformScale };
			j = i + tailSpacingOffset;
			// Don't draw the first sphere because there is an ugly gap. 
			if (i > tailUpdateCount) { break; }
			delta = (tail[j] - translation);
			drawGlSeqTran(sphereList, scale, delta, Eigen::Vector3f{ 1,0,0 }, 0, alpha, bcolor);
		}
	}


	void drawGlSeqOpaq(GLuint list, Eigen::Vector3f scale, Eigen::Vector3f d, Eigen::Vector3f rotAxis, float ang, enum Colors c)
	{
		glPushMatrix();
		glEnable(GL_DEPTH_TEST);
		glColor3f(Colors[c][0], Colors[c][1], Colors[c][2]);
		drawGlSeq(list, scale, d, rotAxis, ang);
	}

	void drawGlSeqTran(GLuint list, Eigen::Vector3f scale, Eigen::Vector3f d, Eigen::Vector3f rotAxis, float ang, float alpha, enum Colors c)
	{
		glPushMatrix();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glColor4f(Colors[c][0], Colors[c][1], Colors[c][2], alpha);
		drawGlSeq(list, scale, d, rotAxis, ang);
	}

	void drawGlSeq(GLuint list, Eigen::Vector3f scale, Eigen::Vector3f d, Eigen::Vector3f rotAxis, float ang)
	{
		glTranslatef(d(0), d(1), d(2));
		glRotatef(ang, rotAxis(0), rotAxis(1), rotAxis(2));
		glScalef(scale(0), scale(1), scale(2));
		glCallList(list);
		glPopMatrix();
	}


	Bodies bType;
	float r_i, m_i, r, m;
	double V;
	long tailLen_i, tailLen;
	state S_i, S;
	Eigen::Vector3f* tail;
	GLuint sphereList, cylinderList, lineList, texture, distortedSphereList;
	BtmStruct bitmap;
	enum Colors bcolor;
	enum Colors tcolor;
	bool selected;
	Eigen::Vector3f lineVec, prevPos;
	GLuint* lineListVec;
	int tailSpacingOffset = 0;
	int tailSpacing;
	long long tailUpdateCount;
	float selectorScale = 1.1f;
	float bodyMinRad = 0.1f;
	int texW, texH;
	char* textPath;
	bool distortion = false;
};




///////////////////////////
//#include "threebodysim.h"
///////////////////////////

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
		distortTex();
		resolveIfContact();
		updateCenter();
		updateTails();
	}

	void distortTex()
	{
		b1.initLists();
	}

	void drawBodies(Views view, Tails tailOption, bool seeTexture)
	{
		Eigen::Vector3f translation;
		if (view == Views::CENTER) { translation = center; }
		else if (view == Views::BODY1) { translation = b1.S.pos; }
		else if (view == Views::BODY2) { translation = b2.S.pos; }
		else { translation = b3.S.pos; }
		// since we are using glScalef( ), be sure normals get unitized:
		glEnable(GL_NORMALIZE);

		if (seeTexture == true) { glEnable(GL_TEXTURE_2D); }
		else { glDisable(GL_TEXTURE_2D); }
		b1.drawObliq(translation, seeTexture);
		//glBindTexture(GL_TEXTURE_2D, b2.texture);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		b2.drawObliq(translation, seeTexture);
		//glBindTexture(GL_TEXTURE_2D, b3.texture);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		b3.drawObliq(translation, seeTexture);
		glDisable(GL_TEXTURE_2D);


		glDepthMask(GL_FALSE);
		b1.drawTran(translation, tailOption);
		b2.drawTran(translation, tailOption);
		b3.drawTran(translation, tailOption);
		boundary.drawBoundary(translation, timeSinceBoundContact);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
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

		glGenTextures(1, &b1.texture); // assign binding “handles”
		glGenTextures(1, &b2.texture); // assign binding “handles”

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




		//b1.initTexture();
		//b2.initTexture();
		//b3.initTexture();
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
		bufferChangeCount += change;
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
		b2.S.vel = b2.S.vel + (b2.S.acc * dt * speed);
		b3.S.vel = b3.S.vel + (b3.S.acc * dt * speed);

		boundary.S.pos = Eigen::Vector3f{ 0,0,0 };
		timeSinceBoundContact += dt;
		b1.prevPos = b1.S.pos;
		b1.S.pos = b1.S.pos + (b1.S.vel * dt * speed);
		b2.prevPos = b2.S.pos;
		b2.S.pos = b2.S.pos + (b2.S.vel * dt * speed);
		b3.prevPos = b3.S.pos;
		b3.S.pos = b3.S.pos + (b3.S.vel * dt * speed);
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
				if (inContact(*bList[i], *bList[j]) == true) { resolveContact(*bList[i], *bList[j]); }
			}
		}
	}

	bool inContact(Body& bodyA, Body& bodyB)
	{
		int dir = findSignf(bodyA.r * bodyB.r);
		if (dir * findDist(bodyA.S.pos, bodyB.S.pos) <= dir * abs((bodyA.r + bodyB.r))) {
			return true;
		}
		else { return false; }
	}

	void resolveContact(Body& bodyA, Body& bodyB)
	{
		float dtAfter = moveToPtOfContact(bodyA, bodyB);
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
			interferance = ((dir * totalRad) - (dir * findDist(ptA, ptB)));
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
		double velA_aft = solveElasticCollision1D(bodyA.m, bodyB.m, velA_bef, velB_bef, coefRest);
		double velB_aft = solveElasticCollision1D(bodyB.m, bodyA.m, velB_bef, velA_bef, coefRest);
		Eigen::Vector3f componentVelAftContactA = velA_aft * findUnit(ContactVecA);
		Eigen::Vector3f componentVelAftContactB = velB_aft * findUnit(ContactVecA);
		bodyA.S.vel = componentVelAftContactA + componentVelOrthoContactA;
		bodyB.S.vel = componentVelAftContactB + componentVelOrthoContactB;
	}


	void resolveRotationalCollision(Body& bodyA, Body& bodyB, Eigen::Vector3f& VelA, Eigen::Vector3f& VelB)
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

	float getCurTime() { return ((float)glutGet(GLUT_ELAPSED_TIME)) / 1000.f; }

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
	Body b1, b2, b3, boundary;
	Body* bList[3] = { &b1,&b2,&b3 };
	Eigen::Vector3f center;
	float speed, curTime;
	float coefRest = 0.5f;
	int bufferChangeCount;
	float timeSinceBoundContact = 100000;
};





/* Reduce tail length or frames per second (FPS) if poor performance. */
const int TAIL_LEN = 20;
const int FPS = 60;
bool useIdle = false;


////// ##################### INITIAL CONDITIONS ##################### //////

///*Edit here*/
//// Body 1
//Eigen::Vector3f b1Pos0 = { -10.0f, 10.0f, -12.0f };
//Eigen::Vector3f b1Vel0 = { -10.0f, 10.0f, -12.0f };
//float b1Rad = 0.5f;
//float b1Mass = 30.0f;
//// Body 2
//Eigen::Vector3f b2Pos0 = { -10.0f, 10.0f, -12.0f };
//Eigen::Vector3f b2Vel0 = { -10.0f, 10.0f, -12.0f };
//float b2Rad = 0.5f;
//float b2Mass = 30.0f;
//// Body 3
//Eigen::Vector3f b3Pos0 = { -10.0f, 10.0f, -12.0f };
//Eigen::Vector3f b3Vel0 = { -10.0f, 10.0f, -12.0f };
//float b3Rad = 0.5f;
//float b3Mass = 30.0f;
///*Stop edit here*/
//
//state b1InitState = { b1Pos0,b1Vel0 ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//state b2InitState = { b2Pos0,b2Vel0 ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//state b3InitState = { b3Pos0,b3Vel0 ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, b1Rad, b1Mass, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//Body b2(1, b2Rad, b2Mass, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//Body b3(2, b3Rad, b3Mass, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);



////// ##################### Pre-Selected Initial Conditions ##################### //////

/* Uncomment below for interesting initial conditions.*/

// //// Nice.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -11.0f) ,Eigen::Vector3f(-3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 10.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 20.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
////
//// // Interesting perfectly balanced conditions. Symmetrical.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
////
////// Different interesting perfectly balanced conditions. Non-symmetrical! Amazing how the center of mass stays constant. Beautiful
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
////
//// Coming directly at you. 
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 5.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 3.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, 1.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
////
//// // Going directly away.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(1.0f, 0.0f, -5.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, -2.0f, -3.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(-1.0f, 2.0f, -1.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
////
////// Going directly right.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(2.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
////
////// Different interesting perfectly balanced conditions. 
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-3.0f, 2.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
// ////
////// Good size to show collisions.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 2.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(2.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 2.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 2.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
//////
////
//////// Test collision. Stationary center.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(Bodies::B1, 2.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(Bodies::B2, 2.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(Bodies::B3, 2.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
//////
 //////// Test collision. Moving center.
state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 4.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b1(Bodies::B1, 2.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b2(Bodies::B2, 2.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b3(Bodies::B3, 2.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
////
////// Test collision. Linear.
//state b1InitState = { Eigen::Vector3f(-1000.0f, 1000.0f, -1200.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(Bodies::B1, 2.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE, FPS);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(Bodies::B2, 2.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN, FPS);
//state b3InitState = { Eigen::Vector3f(30.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(Bodies::B3, 2.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA, FPS);
//////


////// ##################### OBJECTS ##################### //////
state boundaryInitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body boundary(Bodies::BOUNDARY, -70.0f, 3000000.0f, TAIL_LEN, boundaryInitState, Colors::WHITE, Colors::WHITE, FPS);
ThreeBodySim sim(b1, b2, b3, boundary);
//Axis axis(3);


////// ##################### CONSTANT GLOBALS ##################### //////
// title of these windows:
const char *WINDOWTITLE = { "Three Body Beauty -- Kyle Mathenia" };
const char *GLUITITLE   = { "User Interface Window" };
// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };
// how fast to orbit. Units of degrees/frame period.
const float ORBIT_SPEED = 0.5f;
// seconds per frame
const float FRAME_PERIOD = 1 / (float)FPS;
// multiplication factors for input interaction.
const float ANGFACT = { 1. };
const float SCLFACT = { 0.001f };
// minimum allowable scale factor:
const float MINSCALE = { 0.01f };
// window background color (rgba):
const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };
const pt2i SCREEN = GetDesktopResolution();


////// ##################### NON-CONSTANT GLOBALS ##################### //////

int		whichView;
int		whichProjection;
int		whichTail;			
int		axesOn;
int		debugOn;
int		orbitOn;
int		mainWindow;				// window id for main graphics window
float	scale;					// scaling factor
pt2i	mouse;					// mouse location in pixels
pt2f	rot;					// rotation angles in degrees
pt2i	windowSize;				// pixels size of current glut window
float	aspectRatio;			// aspect ratio of the glut window
int		activeButton;			// current button that is down
int		distort;
bool	seeTexture;


////// ##################### FUNCTION PROTOTYPES ##################### //////
// main functions
void	DisplaySetup();
void	Display();
void	Animate();
void	AnimateAtFPS(int);
// init functions
void	InitGraphics();
void	InitLists();
void	InitMenus();
// event callbacks
void	KeyCallback(unsigned char, int, int);
void	MouseButtonCallback(int, int, int, int);
void	MouseMotionCallback(int, int);
void	ResizeCallback(int, int);
void	VisibilityCallback(int);
// menu callbacks
void	DoAxesMenu(int);
void	DoDebugMenu(int);
void	DoOrbitMenu(int);
void	DoViewMenu(int);
void	DoTailMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoResetMenu();
void	DoSoftResetMenu();
void	DoQuitMenu();
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
// ui callbacks
void	DoViewKey();
void	DoTailKey();
void	DoProjectionKey();
void	DoOrbitKey();
void	DoScrollWheel(int upOrDown);
void	DoTextureMenu(int id);


////// ##################### MAIN PROGRAM ##################### //////

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)
	glutInit( &argc, argv );
	// setup all the graphics stuff:
	InitGraphics( );
	// init all the global variables used by Display( ):
	DoResetMenu( );
	// create the display structures that will not change:
	InitLists( );
	// setup all the user interface stuff:
	InitMenus( );
	// draw the scene once and wait for some interaction:
	// (this will never return)
	glutSetWindow( mainWindow );
	glutMainLoop( );
	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:
	return 0;
}


// Moves things that will be drawn in the display function.
void
Animate()
{
	sim.step();
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

// Moves things that will be drawn in the display function at a certain rate. 
void
AnimateAtFPS(int)
{
	// The actual frame period is not always constant. Need to adjust with dt/frame_time ratio, otherwise jerky orbiting. 
	if (orbitOn == 1) { rot.y += -ORBIT_SPEED * (sim.dt / FRAME_PERIOD); }
	Animate();
	glutTimerFunc(1000 / FPS, AnimateAtFPS, 0);
}

// Draw the complete scene. Happening all the time, very fast, in the glutMainLoop()
void
Display()
{
	DisplaySetup();
	// place the objects into the scene:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set the eye position, look-at position, and up-vector:
	gluLookAt(0., 0., 21., 0., 0., 0., 0., 1., 0.);
	// draw the axis before any global transformations.
	if (axesOn == 1) {
		pt3i axisTranslation = { -10, -10, 8 };
		//axis.draw(rot, axisTranslation);
	}
	// uniformly scale the scene:
	if (scale < MINSCALE)
		scale = MINSCALE;
	glScalef((GLfloat)scale, (GLfloat)scale * aspectRatio, (GLfloat)scale);
	// rotate the scene:
	glRotatef((GLfloat)rot.y, 0., 1., 0.);
	glRotatef((GLfloat)rot.x, 1., 0., 0.);
	// draw the bodies
	sim.drawBodies((Views)whichView, (Tails)whichTail, seeTexture);
	// finish
	glutSwapBuffers();
	glFlush();
  }



// Set some glut and opengl parameters every display call. 
void
DisplaySetup()
{
	// set which window we want to do the graphics into
	glutSetWindow(mainWindow);
	// erase the background
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	// specify shading to be flat:
	glShadeModel(GL_FLAT);
	// set the viewport to a square centered in the window:
	windowSize.x = glutGet(GLUT_WINDOW_WIDTH);
	windowSize.y = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, windowSize.x, windowSize.y);
	aspectRatio = (float)windowSize.x / (float)windowSize.y;
	// set the viewing volume:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (whichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 10000.);
	else
		gluPerspective(90, 1., 0.1, 10000.);
}


////// ##################### INIT FUNCTIONS ##################### //////

// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
void
InitGraphics()
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	// set the initial window configuration:
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(SCREEN.x, SCREEN.y);
	// open the window and set its title:
	mainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);
	// set the framebuffer clear values:
	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);
	// set glut callbacks
	glutSetWindow(mainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(ResizeCallback);
	glutKeyboardFunc(KeyCallback);
	glutMouseFunc(MouseButtonCallback);
	glutMotionFunc(MouseMotionCallback);
	glutPassiveMotionFunc(MouseMotionCallback);
	glutVisibilityFunc(VisibilityCallback);
	if (useIdle == true) {
		// Animate as fast as possible all the time.
		glutTimerFunc(-1, NULL, 0);
		glutIdleFunc(Animate);
	}
	else {
		// Use the timer to make sure the tail spacing is somewhat consistent. 
		glutTimerFunc(1, AnimateAtFPS, 0);
		glutIdleFunc(NULL);
	}
	// init glew (a window must be open to do this):
#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
	glutFullScreen();


	unsigned char* textarr1 = BmpToTexture((char*)"C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\textures\\worldtex.bmp", &sim.b1.texW, &sim.b1.texH);
	//unsigned char* textarr2 = BmpToTexture((char*)"C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\textures\\worldtex.bmp", &sim.b2.texW, &sim.b2.texH);
	//unsigned char* textarr3 = BmpToTexture((char*)"C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\textures\\worldtex.bmp", &sim.b3.texW, &sim.b3.texH);
	//unsigned char* textarrBound = BmpToTexture((char*)"C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\textures\\Starsinthesky11.bmp", &sim.boundary.texW, &sim.boundary.texH);
	//
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &sim.b1.texture); // assign binding “handles”
	//glGenTextures(1, &sim.b2.texture); // assign binding “handles”
	//glGenTextures(1, &sim.b3.texture); // assign binding “handles”
	//glGenTextures(1, &sim.boundary.texture); // assign binding “handles”

	glBindTexture(GL_TEXTURE_2D, sim.b1.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, sim.b1.texW, sim.b1.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, textarr1);

	//glBindTexture(GL_TEXTURE_2D, sim.b2.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, sim.b2.texW, sim.b2.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, textarrBound);

	//glBindTexture(GL_TEXTURE_2D, sim.b3.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, sim.b3.texW, sim.b3.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, textarrBound);

	//glBindTexture(GL_TEXTURE_2D, sim.boundary.texture); // make the Tex0 texture current and set its parametersglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, sim.boundary.texW, sim.boundary.texH, 0, GL_RGB, GL_UNSIGNED_BYTE, textarrBound);
}


void
InitMenus()
{
	glutSetWindow(mainWindow);
	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);
	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);
	int orbitmenu = glutCreateMenu(DoOrbitMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);
	int texturemenu = glutCreateMenu(DoTextureMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);
	glutAddMenuEntry("Distort", 2);
	int viewmenu = glutCreateMenu(DoViewMenu);
	glutAddMenuEntry("Center", (int)Views::CENTER);
	glutAddMenuEntry("Body 1", (int)Views::BODY1);
	glutAddMenuEntry("Body 2", (int)Views::BODY2);
	glutAddMenuEntry("Body 3", (int)Views::BODY3);
	int tailmenu = glutCreateMenu(DoTailMenu);
	glutAddMenuEntry("Spheres", (int)Tails::SPHERES);
	glutAddMenuEntry("Cylinders", (int)Tails::CYLINDERS);
	glutAddMenuEntry("Line", (int)Tails::LINES);
	glutAddMenuEntry("None", (int)Tails::NONE);
	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);
	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Texture", texturemenu);
	glutAddSubMenu("View (space)", viewmenu);
	glutAddSubMenu("Tail (t)", tailmenu);
	glutAddSubMenu("Orbit (o)", orbitmenu);
	glutAddSubMenu("Projection (p)", projmenu);
	glutAddMenuEntry("Soft Reset (r)", SOFT_RESET);
	glutAddMenuEntry("Reset (R)", RESET);
	glutAddMenuEntry("Quit (q/esc)", QUIT);
	// attach the pop-up menu to the right mouse button:
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// initialize the display lists.
void
InitLists()
{
	glutSetWindow(mainWindow);
	sim.initLists();
	//axis.initList();
}


////// ##################### GLUT CALLBACKS ##################### //////

// called when any key is pressed
void
KeyCallback(unsigned char c, int x, int y)
{
	if (debugOn != 0) { fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c); }

	if (c == 'p') { DoProjectionKey(); }
	else if (c == 'o') { DoOrbitKey(); }
	else if (c == ' '){ DoViewKey(); }
	else if (c == 't') { DoTailKey(); }
	else if (c == 'n') { glutReshapeWindow(SCREEN.x, SCREEN.y); }
	else if (c == 'r'){ DoSoftResetMenu(); }
	else if (c == 'R') { DoResetMenu(); }
	else if (c == 'd') { sim.changeSpeed(0.5f); }
	else if (c == 'e') { sim.changeSpeed(2.0f); }
	else if (c == 's') { sim.changeSelected(-1); }
	else if (c == 'f') { sim.changeSelected(1); }
	else if (c == 'g') { sim.changeSize(1.0526316f); }
	else if (c == 'a') { sim.changeSize(0.95f); }
	else if (c == 'q' || c == EventEnums::ESCAPE) { DoMainMenu(QUIT); }
	else { fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c); }

	// force a call to Display( ):
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}


// called when the mouse button transitions down or up
void
MouseButtonCallback(int button, int state, int x, int y)
{
	if (debugOn != 0) { fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y); }

	int b = 0;			// LEFT, MIDDLE, or RIGHT
	// get the proper button bit mask:
	if (button == GLUT_LEFT_BUTTON){ b = EventEnums::LEFT; }
	else if (button == GLUT_MIDDLE_BUTTON) { b = EventEnums::MIDDLE; }
	else if (button == GLUT_RIGHT_BUTTON) { b = EventEnums::RIGHT; }
	else if (button == EventEnums::SCROLL_WHEEL_UP) {DoScrollWheel(1);}
	else if (button == EventEnums::SCROLL_WHEEL_DOWN) { DoScrollWheel(-1); }
	else {fprintf(stderr, "Unknown mouse button: %d\n", button);}

	// button down sets the bit, up clears the bit:
	if (state == GLUT_DOWN) {
		mouse.x = x;
		mouse.y = y;
		activeButton |= b;		// set the proper bit
	}
	else {
		activeButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(mainWindow);
	glutPostRedisplay();
}


// called when the mouse moves while a button is down
void
MouseMotionCallback(int x, int y)
{
	if (debugOn != 0) { fprintf(stderr, "MouseMotion: %d, %d\n", x, y); }

	// change in mouse coords
	int dx = x - mouse.x;
	int dy = y - mouse.y;
	if ((activeButton & EventEnums::LEFT) != 0)
	{
		rot.x += (ANGFACT * dy);
		rot.y += (ANGFACT * dx);
	}
	if ((activeButton & EventEnums::MIDDLE) != 0)
	{
		scale += SCLFACT * (float)(dx - dy);
		// keep object from turning inside-out or disappearing:
		if (scale < MINSCALE)
			scale = MINSCALE;
	}
	// new current position
	mouse.x = x;			
	mouse.y = y;

	glutSetWindow(mainWindow);
	glutPostRedisplay();
}


// called when user resizes the window:
void
ResizeCallback(int width, int height)
{
	if (debugOn != 0) { fprintf(stderr, "ReSize: %d, %d\n", width, height); }
	// don't really need to do anything since the window size is checked each time in Display( )
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:
void
VisibilityCallback(int state)
{
	if (debugOn != 0) { fprintf(stderr, "Visibility: %d\n", state); }

	if (state == GLUT_VISIBLE) {
		glutSetWindow(mainWindow);
		glutPostRedisplay();
	}
	else {
		// could optimize by keeping track of the fact that the window is not visible and avoid
		// animating or redrawing it ...
	}
}


////// ##################### MENU CALLBACKS ##################### //////

// This is where initial variables are set
void
DoResetMenu()
{
	activeButton = 0;
	axesOn = 0;
	debugOn = 0;
	orbitOn = 0;
	scale = 1.0f;
	whichProjection = PERSP;
	whichView = (int)Views::CENTER;
	whichTail = (int)Tails::LINES;
	rot.x = rot.y = 0;
	seeTexture = true;
	sim.reset();
}

void
DoMainMenu(int id)
{
	if (id == RESET){ DoResetMenu(); }
	else if (id == SOFT_RESET) { DoSoftResetMenu(); }
	else if (id == QUIT) { DoQuitMenu(); }
	else { fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id); }

	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

void
DoAxesMenu( int id )
{
	axesOn = id;
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

void
DoDebugMenu( int id )
{
	debugOn = id;
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

void
DoOrbitMenu(int id)
{
	orbitOn = id;
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

void
DoViewMenu(int id)
{
	whichView = id;
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

void
DoTextureMenu(int id)
{
	if (id == 0) {
		sim.b1.distortion = false;
		seeTexture = false;
	}
	else if (id == 1) {
		sim.b1.distortion = false;
		seeTexture = true;
	}
	else {
		sim.b1.distortion = true;
		seeTexture = true;
	}
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

void
DoTailMenu(int id)
{
	whichTail = id;
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

void
DoProjectMenu( int id )
{
	whichProjection = id;
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

void
DoSoftResetMenu()
{
	sim.reset();
}

void
DoQuitMenu()
{
	glutSetWindow(mainWindow);
	glFinish();
	glutDestroyWindow(mainWindow);
	exit(0);
}


////// ##################### UI CALLBACKS ##################### //////

void
DoViewKey()
{
	static int count = 0;
	count++;
	whichView = count % int(Views::MAX_NUM_VIEWS + 1);
}

void
DoTailKey()
{
	static int count = 0;
	count++;
	whichTail = count % int(Tails::NONE + 1);
}

void
DoProjectionKey()
{
	if (whichProjection == PERSP) { whichProjection = ORTHO; }
	else { whichProjection = PERSP; }
}

void
DoOrbitKey()
{
	if (orbitOn == 0) { orbitOn = 1; }
	else { orbitOn = 0; }
}

void
DoScrollWheel(int upOrDown)
{
	scale += (SCLFACT * EventEnums::SCROLL_WHEEL_CLICK_FACTOR) * upOrDown;
	// keep object from turning inside-out or disappearing:
	if (scale < MINSCALE)
		scale = MINSCALE;
}
