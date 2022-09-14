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


bool usePresetConditions = false;
// Reduce tail length if poor performance
const int TAIL_LEN = 500;

////// ##################### Initial Conditions ##################### //////

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


////// ##################### Objects ##################### //////

ThreeBodySim sim(b1, b2, b3);
Axis axis(3);


////// ##################### Const Globals ##################### //////
// title of these windows:
const char *WINDOWTITLE = { "Three Body Beauty -- Kyle Mathenia" };
const char *GLUITITLE   = { "User Interface Window" };
// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };
// how fast to orbit. Units of degrees/frame period.
const float ORBIT_SPEED = 0.2f;
// frames per second
const int FPS = 60;
// seconds per frame
const float FRAME_PERIOD = 1 / (float)FPS;
// multiplication factors for input interaction:
//  (these are known from previous experience)S
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };
// minimum allowable scale factor:
const float MINSCALE = { 0.05f };
// window background color (rgba):
const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


////// ##################### Non-const Globals ##################### //////

int		whichView;
int		whichProjection;
int		axesOn;
int		debugOn;
int		orbitOn;
screenSize screen;
int		mainWindow;				// window id for main graphics window
float	scale;					// scaling factor
int		whichColor;				// index into Colors[ ]
int		mouseX, mouseY;			// mouse values
float	rotX, rotY;				// rotation angles in degrees
float	aspectRatio;			// aspect ratio of the glut window
int		tailOption;				// option selected for tail
int		activeButton;			// current button that is down


// function prototypes:
void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDebugMenu( int );
void	DoOrbitMenu( int );
void	DoViewMenu( int );
void	DoTailMenu(int);
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoShadowMenu();
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	ChangeLists(int);
void	ChangeLists2();
void	InitMenus( );
void	changeView();
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	DoResetMenu( );
void	DoSoftResetMenu();
void	Resize( int, int );
void	Visibility( int );

void	Axes( float );


///////////////////////////////////////   Main Program:  //////////////////////////
////// ##################### KEYBOARD CALLBACKS ##################### //////

// main program:
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

// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it
//glutIdleFunc(Animate) would crash the program after ~20 seconds. Something was going wrong with glutIdleFunc, but glutTimerFunc works. 
void
Animate( )
{
	sim.step();
	if (orbitOn == 1) { 
		// The frame time is not always constant. Need to adjust with dt/frame_time ratio, otherwise jerky orbiting. 
		rotY += -ORBIT_SPEED * (sim.dt/FRAME_PERIOD);
	}
	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

void
AnimateAtFPS(int)
{
	Animate();
	glutTimerFunc(1000 / FPS, AnimateAtFPS, 0);
}

// draw the complete scene:
void
Display( )
{
	// set which window we want to do the graphics into:
	glutSetWindow( mainWindow );
	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );

	// specify shading to be flat:
	glShadeModel( GL_FLAT );

	// set the viewport to a square centered in the window:
	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(0, 0, vx, vy);
	aspectRatio = (float)vx / (float)vy;

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( whichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );

	// place the objects into the scene:
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:
	gluLookAt( 0., 0., 21.,     0., 0., 0.,     0., 1., 0. );
	//glTranslatef((float)sim.center(0), (float)sim.center(1), (float)sim.center(2));
	//gluLookAt( 0., 0., 8.,     0., 8., 0.,     0., 1., 0. );
	//gluLookAt((float)sim.center(0), (float)sim.center(1), (float)sim.center(2) + 8, (float)sim.center(0), (float)sim.center(1), (float)sim.center(2), 0., 1., 0.);
	//glTranslatef((float)sim.center(0), (float)sim.center(1), (float)sim.center(2));

	//axis.draw(Yrot,Xrot);

	//axis.draw();

	// uniformly scale the scene:
	if( scale < MINSCALE )
		scale = MINSCALE;
	glScalef( (GLfloat)scale, (GLfloat)scale*aspectRatio, (GLfloat)scale);

	// rotate the scene:
	glRotatef((GLfloat)rotY, 0., 1., 0.);
	glRotatef((GLfloat)rotX, 1., 0., 0.);


	//axis.draw();
	sim.drawBodies((Views)whichView, (Tails)tailOption);

	 //draw some gratuitous text that just rotates on top of the scene:
	glDisable( GL_DEPTH_TEST );
	glColor3f( 0., 1., 1. );
	DoRasterString( 0., 1., 0., (char *)"Text That Moves" );
	
	// swap the double-buffered framebuffers:
	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !
	glFlush( );
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
	rotX = rotY = 0.;
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
changeView()
{
	static int count = 0;
	count++;
	whichView = count % int(Views::MAX_NUM_VIEWS + 1);
}

// return the number of seconds since the start of the program:
float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:
	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:
	return (float)ms / 1000.f;
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


// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
void
InitGraphics( )
{
	screen = GetDesktopResolution();
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:
	glutInitWindowPosition( 0, 0 );
	//glutInitWindowSize( INIT_WINDOW_SIZE+300, INIT_WINDOW_SIZE);
	//glutInitWindowSize(initWindowX, initWindowY);
	glutInitWindowSize(screen.x,screen.y);

	// open the window and set its title:
	mainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );
	//glutFullScreen();

	// set the framebuffer clear values:
	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( mainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	// Using the timer to make sure the tail lengths don't change a bunch depending on the refresh rate. 
	glutTimerFunc( 1, AnimateAtFPS, 0 );
	//glutTimerFunc(-1, NULL, 0);
	//glutIdleFunc(Animate);
	glutIdleFunc(NULL);

	// init glew (a window must be open to do this):
#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
}

// the keyboard callback:
void
Keyboard( unsigned char c, int x, int y )
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
			changeView();
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
MouseButton( int button, int state, int x, int y )
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
		mouseX = x;
		mouseY = y;
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
MouseMotion( int x, int y )
{
	if( debugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );

	int dx = x - mouseX;		// change in mouse coords
	int dy = y - mouseY;

	if( ( activeButton & EventEnums::LEFT ) != 0 )
	{
		rotX += ( ANGFACT*dy );
		rotY += ( ANGFACT*dx );
	}

	if( ( activeButton & EventEnums::MIDDLE ) != 0 )
	{
		scale += SCLFACT * (float) ( dx - dy );
		// keep object from turning inside-out or disappearing:
		if( scale < MINSCALE )
			scale = MINSCALE;
	}

	mouseX = x;			// new current position
	mouseY = y;

	glutSetWindow( mainWindow );
	glutPostRedisplay( );
}

// called when user resizes the window:
void
Resize( int width, int height )
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
Visibility ( int state )
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