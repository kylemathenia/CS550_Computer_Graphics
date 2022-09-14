// standard
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <chrono>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
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

// src
#include "utils.h"
#include "color.h"
#include "options.h"
#include "Shapes/shapes.h"
#include "body.h"
#include "axis.h"
#include "threebodysim.h"


/* Reduce tail length or frames per second (FPS) if poor performance. */
const int TAIL_LEN = 500;
const int FPS = 60;

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
//Body b1(0, b1Rad, b1Mass, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE);
//Body b2(1, b2Rad, b2Mass, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN);
//Body b3(2, b3Rad, b3Mass, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA);



////// ##################### Pre-Selected Initial Conditions ##################### //////

/* Uncomment below for interesting initial conditions.*/

// //// Nice.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -11.0f) ,Eigen::Vector3f(-3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 10.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 20.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA);
////
//// // Interesting perfectly balanced conditions. Symmetrical.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA);
////
////// Different interesting perfectly balanced conditions. Non-symmetrical! Amazing how the center of mass stays constant. Beautiful
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA);
////
//// Coming directly at you. 
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 5.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 3.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, 1.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA);
//
//// Going directly right.
state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b1(0, 0.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE);
state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(2.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b2(1, 0.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN);
state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b3(2, 0.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA);
////
////// Going directly right. Good size to show collisions.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 2.5f, 30.0f, TAIL_LEN, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(2.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 2.5f, 30.0f, TAIL_LEN, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 2.5f, 30.0f, TAIL_LEN, b3InitState, Colors::RED, Colors::MAGENTA);
////
////// Different interesting perfectly balanced conditions. 
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-3.0f, 2.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);


////// ##################### OBJECTS ##################### //////

ThreeBodySim sim(b1, b2, b3);
Axis axis(3);


////// ##################### CONSTANT GLOBALS ##################### //////
// title of these windows:
const char *WINDOWTITLE = { "Three Body Beauty -- Kyle Mathenia" };
const char *GLUITITLE   = { "User Interface Window" };
// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };
// how fast to orbit. Units of degrees/frame period.
const float ORBIT_SPEED = 0.2f;
// seconds per frame
const float FRAME_PERIOD = 1 / (float)FPS;
// multiplication factors for input interaction.
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };
// minimum allowable scale factor:
const float MINSCALE = { 0.05f };
// window background color (rgba):
const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


////// ##################### NON-CONSTANT GLOBALS ##################### //////

int		whichView;
int		whichProjection;
int		axesOn;
int		debugOn;
int		orbitOn;
int		mainWindow;				// window id for main graphics window
float	scale;					// scaling factor
int		whichColor;				// index into Colors[ ]
pt2i	mouse;					// mouse location in pixels
pt2f	rot;					// rotation angles in degrees
float	aspectRatio;			// aspect ratio of the glut window
int		tailOption;				// option selected for tail
int		activeButton;			// current button that is down


////// ##################### FUNCTION PROTOTYPES ##################### //////
// main functions
void	InitGraphics();
void	InitLists();
void	InitMenus();
void	Display( );
void	Animate();
void	AnimateAtFPS(int);
// ui callbacks
void	KeyCallback(unsigned char, int, int);
void	MouseButtonCallback(int, int, int, int);
void	MouseMotionCallback(int, int);
void	ResizeCallback(int, int);
void	VisibilityCallback(int);
// menu callbacks
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDebugMenu(int);
void	DoOrbitMenu(int);
void	DoViewMenu(int);
void	DoTailMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoResetMenu();
void	DoSoftResetMenu();
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
// keyboard callbacks
void	DoViewKey();


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
	// set which window we want to do the graphics into
	glutSetWindow(mainWindow);
	// erase the background
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set the viewport to a square centered in the window:
	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, vx, vy);
	aspectRatio = (float)vx / (float)vy;

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (whichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);

	// place the objects into the scene:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:
	gluLookAt(0., 0., 21., 0., 0., 0., 0., 1., 0.);
	//glTranslatef((float)sim.center(0), (float)sim.center(1), (float)sim.center(2));
	//gluLookAt( 0., 0., 8.,     0., 8., 0.,     0., 1., 0. );
	//gluLookAt((float)sim.center(0), (float)sim.center(1), (float)sim.center(2) + 8, (float)sim.center(0), (float)sim.center(1), (float)sim.center(2), 0., 1., 0.);
	//glTranslatef((float)sim.center(0), (float)sim.center(1), (float)sim.center(2));

	//axis.draw(Yrot,Xrot);

	//axis.draw();

	// uniformly scale the scene:
	if (scale < MINSCALE)
		scale = MINSCALE;
	glScalef((GLfloat)scale, (GLfloat)scale * aspectRatio, (GLfloat)scale);

	// rotate the scene:
	glRotatef((GLfloat)rot.y, 0., 1., 0.);
	glRotatef((GLfloat)rot.x, 1., 0., 0.);


	//axis.draw();
	sim.drawBodies((Views)whichView, (Tails)tailOption);

	//draw some gratuitous text that just rotates on top of the scene:
	glDisable(GL_DEPTH_TEST);
	glColor3f(0., 1., 1.);
	DoRasterString(0., 1., 0., (char*)"Text That Moves");

	// swap the double-buffered framebuffers:
	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !
	glFlush();
}

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
	pt2i screen = GetDesktopResolution();
	glutInitWindowSize(screen.x, screen.y);
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
	if (FPS != 0) {
		// Using the timer to make sure the tail lengths don't change a bunch depending on the refresh rate. 
		glutTimerFunc(1, AnimateAtFPS, 0);
		glutIdleFunc(NULL);
	}
	else {
		// Animate as fast as possible all the time.
		glutTimerFunc(-1, NULL, 0);
		glutIdleFunc(Animate);
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
}

// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )
void
InitLists()
{
	glutSetWindow(mainWindow);
	sim.initLists();
	axis.initList();
}



////// ##################### MENU CALLBACKS ##################### //////

void
DoAxesMenu( int id )
{
	axesOn = id;
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

void
DoColorMenu( int id )
{
	whichColor = id - RED;
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
DoTailMenu(int id)
{
	tailOption = id;
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

// main menu callback:
void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			DoResetMenu( );
			break;
		
		case SOFT_RESET:
			DoSoftResetMenu();
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( mainWindow );
			glFinish( );
			glutDestroyWindow( mainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

void
DoProjectMenu( int id )
{
	whichProjection = id;
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

void
DoResetMenu()
{
	activeButton = 0;
	axesOn = 1;
	debugOn = 1;
	scale = 1.0;
	whichProjection = PERSP;
	rot.x = rot.y = 0;
	sim.reset();
}

void
DoSoftResetMenu()
{
	sim.reset();
}

// use glut to display a string of characters using a raster font:
void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}

// use glut to display a string of characters using a stroke font:
void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


////// ##################### KEYBOARD CALLBACKS ##################### //////

void
DoViewKey()
{
	static int count = 0;
	count++;
	whichView = count % int(Views::MAX_NUM_VIEWS + 1);
}

// initialize the glui window:
void
InitMenus( )
{
	glutSetWindow( mainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int orbitmenu = glutCreateMenu(DoOrbitMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int viewmenu = glutCreateMenu(DoViewMenu);
	glutAddMenuEntry("Center", (int)Views::CENTER);
	glutAddMenuEntry("Body 1", (int)Views::BODY1);
	glutAddMenuEntry("Body 2", (int)Views::BODY2);
	glutAddMenuEntry("Body 3", (int)Views::BODY3);

	int tailmenu = glutCreateMenu(DoTailMenu);
	glutAddMenuEntry("None", (int)Tails::NONE);
	glutAddMenuEntry("Con Thick Line", (int)Tails::CONST_THICK_LINE);
	glutAddMenuEntry("Var Thick Line", (int)Tails::VAR_THICK_LINE);
	glutAddMenuEntry("Cylinders", (int)Tails::CYLINDERS);
	glutAddMenuEntry("Spheres", (int)Tails::SPHERES);

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );

	glutAddSubMenu("View (space)", viewmenu);
	glutAddSubMenu("Tail (t)", tailmenu);
	glutAddSubMenu("Orbit (o)", orbitmenu);
	glutAddSubMenu(   "Projection (p)",    projmenu );
	glutAddMenuEntry("Soft Reset (r)", SOFT_RESET);
	glutAddMenuEntry( "Reset (R)",         RESET );
	glutAddMenuEntry( "Quit (q/esc)",          QUIT );

// attach the pop-up menu to the right mouse button:
	glutAttachMenu( GLUT_RIGHT_BUTTON );
}

// the keyboard callback:
void
KeyCallback( unsigned char c, int x, int y )
{
	if( debugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'p':
			if (whichProjection == PERSP) { whichProjection = ORTHO; }
			else { whichProjection = PERSP; }
			break;

		case ' ':
			DoViewKey();
			break;
		
		case 'r':
			sim.reset();
			break;

		case 'o':
			if (orbitOn == 0) { orbitOn = 1; }
			else { orbitOn = 0; }
			break;

		case 'd':
			sim.changeSpeed(0.5f);
			break;
		case 'e':
			sim.changeSpeed(2.0f);
			break;

		case 's':
			sim.changeSelected(-1);
			break;
		case 'f':
			sim.changeSelected(1);
			break;

		case 'g':
			sim.changeSize(0.05f);
			break;
		case 'a':
			sim.changeSize(-0.05f);
			break;

		case 'q':
		case EventEnums::ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:
void
MouseButtonCallback( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( debugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	// get the proper button bit mask:
	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = EventEnums::LEFT;		break;
		case GLUT_MIDDLE_BUTTON:
			b = EventEnums::MIDDLE;		break;
		case GLUT_RIGHT_BUTTON:
			b = EventEnums::RIGHT;		break;
		case EventEnums::SCROLL_WHEEL_UP:
			scale += SCLFACT * EventEnums::SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (scale < MINSCALE)
				scale = MINSCALE;
			break;
		case EventEnums::SCROLL_WHEEL_DOWN:
			scale -= SCLFACT * EventEnums::SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (scale < MINSCALE)
				scale = MINSCALE;
			break;
		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:
	if( state == GLUT_DOWN ){
		mouse.x = x;
		mouse.y = y;
		activeButton |= b;		// set the proper bit
	} 
	else{
		activeButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(mainWindow);
	glutPostRedisplay();
}


// called when the mouse moves while a button is down:
void
MouseMotionCallback( int x, int y )
{
	if( debugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );

	int dx = x - mouse.x;		// change in mouse coords
	int dy = y - mouse.y;

	if( ( activeButton & EventEnums::LEFT ) != 0 )
	{
		rot.x += ( ANGFACT*dy );
		rot.y += ( ANGFACT*dx );
	}

	if( ( activeButton & EventEnums::MIDDLE ) != 0 )
	{
		scale += SCLFACT * (float) ( dx - dy );
		// keep object from turning inside-out or disappearing:
		if( scale < MINSCALE )
			scale = MINSCALE;
	}

	mouse.x = x;			// new current position
	mouse.y = y;

	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

// called when user resizes the window:
void
ResizeCallback( int width, int height )
{
	if( debugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );
	// don't really need to do anything since the window size is
	// checked each time in Display( ):
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:
void
VisibilityCallback ( int state )
{
	if( debugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE ){
		glutSetWindow( mainWindow );
		glutPostRedisplay( );
	}
	else{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}