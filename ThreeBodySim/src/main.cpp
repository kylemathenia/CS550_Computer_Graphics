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

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "Eigen/Dense"


// I split out utility functions to other files.  
#include "utils.h"
#include "color.h"
#include "options.h"
// I moved shapes, like osusphere, to a shapes dir and made a common header.
#include "Shapes/shapes.h"
// I created some classes.
#include "body.h"
#include "axis.h"
#include "threebodysim.h"


///////////////////////////////////////   SETUP:  //////////////////////////

// title of these windows:
const char *WINDOWTITLE = { "OpenGL / GLUT Sample -- Joe Graphics" };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:
const int ESCAPE = { 0x1b };

// initial window size:
const int INIT_WINDOW_SIZE = { 600 };

// size of the 3d box:
float BOXSIZE = { 2.f };
const float ORIG_BOXSIZE = BOXSIZE;

//frames per second
const int FPS = 45;

// multiplication factors for input interaction:
//  (these are known from previous experience)S
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:
const float MINSCALE = { 0.05f };

// scroll wheel button values:
const int SCROLL_WHEEL_UP   = { 3 };
const int SCROLL_WHEEL_DOWN = { 4 };

// equivalent mouse movement when we click a the scroll wheel:
const float SCROLL_WHEEL_CLICK_FACTOR = { 5. };

// active mouse buttons (or them together):
const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

// window background color (rgba):
const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// fog parameters:
const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// non-constant global variables:
int		view;					// current view
int		ActiveButton;			// current button that is down
GLuint	axesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	boxList;				// object display list
GLuint	sphereList;
int		DebugOn;				// != 0 means to print debugging info
int		OrbitOn;				// != 0 means the view will orbit at a const rate
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
float	boxScale;				// scalling factor for the box.
float	Time;					// timer in the range [0.,1.)
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

//// class instances:
// 
// Some nice initial conditions. 
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -11.0f) ,Eigen::Vector3f(-3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 10.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 20.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);

//// // Interesting perfectly balanced conditions. Symmetrical.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);

////// Different interesting perfectly balanced conditions. Non-symmetrical! Amazing how the center of mass stays constant. Beautiful
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);

//// Coming directly at you. 
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-1.0f, 0.0f, 5.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 2.0f, 3.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(1.0f, -2.0f, 1.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);

//// Going directly right.
state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b1(0, 0.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(2.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b2(1, 0.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);

////// Going directly right. Good size to show collisions.
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(1.0f, 0.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 2.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(2.0f, 2.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 2.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 2.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);

////// Different interesting perfectly balanced conditions. 
//state b1InitState = { Eigen::Vector3f(-10.0f, 10.0f, -12.0f) ,Eigen::Vector3f(-3.0f, 2.0f, 2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b1(0, 0.5f, 30.0f, 1000, b1InitState, Colors::BLUE, Colors::WHITE);
//state b2InitState = { Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b2(1, 0.5f, 30.0f, 1000, b2InitState, Colors::CYAN, Colors::GREEN);
//state b3InitState = { Eigen::Vector3f(10.0f, 10.0f, 12.0f) ,Eigen::Vector3f(3.0f, -2.0f, -2.0f) ,Eigen::Vector3f(0.0f, 0.0f, 0.0f) };
//Body b3(2, 0.5f, 30.0f, 1000, b3InitState, Colors::RED, Colors::MAGENTA);


ThreeBodySim sim(b1, b2, b3);
Axis axis(3);

// function prototypes:
void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoOrbitMenu( int );
void	DoViewMenu( int );
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
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
unsigned char *	BmpToTexture( char *, int *, int * );
void			HsvRgb( float[3], float [3] );
int				ReadInt( FILE * );
short			ReadShort( FILE * );

void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);




///////////////////////////////////////   Main Program:  //////////////////////////

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
	Reset( );
	// create the display structures that will not change:
	InitLists( );
	// setup all the user interface stuff:
	InitMenus( );
	// draw the scene once and wait for some interaction:
	// (this will never return)
	glutSetWindow( MainWindow );
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
	glutSetWindow(MainWindow);
	// create the objects:
	// If adding more than one object, need to create multiple seperate lists...
	boxList = getCubeList(BOXSIZE);
	//b1.m_initList();
	sim.initLists();
	//sphereList = getSphereList(0.5f, 40, 40);
	axis.initList();
	//axesList = getAxesList(AXES_WIDTH);
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
	//const int MS_IN_THE_ANIMATION_CYCLE = 10000;	// milliseconds in the animation loop
	//int ms = glutGet(GLUT_ELAPSED_TIME);			// milliseconds since the program started
	//ms %= MS_IN_THE_ANIMATION_CYCLE;				// milliseconds in the range 0 to MS_IN_THE_ANIMATION_CYCLE-1
	//Time = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;        // [ 0., 1. )

	//sim.step()

	boxScale = (float)cos(ElapsedSeconds()) + 1 + (1 / 10);
	sim.step();

	if (OrbitOn == 1) { 
		Yrot += -.6;
	}

	// force a call to Display( ) next time it is convenient:
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
Animate2(int)
{
	Animate();
	glutTimerFunc(1000 / FPS, Animate2, 0);
}

// draw the complete scene:
void
Display( )
{
	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );
	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif

	// specify shading to be flat:
	glShadeModel( GL_FLAT );

	// set the viewport to a square centered in the window:
	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );

	// place the objects into the scene:
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:
	gluLookAt( 0., 0., 8.,     0., 0., 0.,     0., 1., 0. );
	//glTranslatef((float)sim.center(0), (float)sim.center(1), (float)sim.center(2));
	//gluLookAt( 0., 0., 8.,     0., 8., 0.,     0., 1., 0. );
	//gluLookAt((float)sim.center(0), (float)sim.center(1), (float)sim.center(2) + 8, (float)sim.center(0), (float)sim.center(1), (float)sim.center(2), 0., 1., 0.);
	//glTranslatef((float)sim.center(0), (float)sim.center(1), (float)sim.center(2));

	//axis.draw(Yrot,Xrot);

	// rotate the scene:
	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );

	//axis.draw();

	// uniformly scale the scene:
	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:
	// (this is really here to do intensity depth cueing)
	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	//axis.draw();
	sim.drawBodies((Views)view);

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.,   0., 1., 0. );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif

	// draw some gratuitous text that just rotates on top of the scene:
	//glDisable( GL_DEPTH_TEST );
	//glColor3f( 0., 1., 1. );
	//DoRasterString( 0., 1., 0., (char *)"Text That Moves" );

	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	//glDisable( GL_DEPTH_TEST );
	//glMatrixMode( GL_PROJECTION );
	//glLoadIdentity( );
	//gluOrtho2D( 0., 100.,     0., 100. );
	//glMatrixMode( GL_MODELVIEW );
	//glLoadIdentity( );
	//glColor3f( 1., 1., 1. );
	//DoRasterString( 5., 5., 0., (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:
	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !
	glFlush( );
}

void
DoAxesMenu( int id )
{
	AxesOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoColorMenu( int id )
{
	WhichColor = id - RED;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoDebugMenu( int id )
{
	DebugOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoOrbitMenu(int id)
{
	OrbitOn = id;
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoViewMenu(int id)
{
	view = id;
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoDepthMenu( int id )
{
	DepthCueOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// main menu callback:
void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoProjectMenu( int id )
{
	WhichProjection = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
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
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
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

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Colors",        colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddSubMenu(	  "Orbit",         orbitmenu);
	glutAddSubMenu(   "View",          viewmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:
	glutAttachMenu( GLUT_RIGHT_BUTTON );
}

// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:
	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

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

	glutSetWindow( MainWindow );
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
	glutTimerFunc( 1, Animate2, 0 );
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
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'p':
			if (WhichProjection == PERSP) { WhichProjection = ORTHO; }
			else { WhichProjection = PERSP; }
			break;
		
		case 'r':
			sim.reset();
			break;

		case 'o':
			if (OrbitOn == 0) { OrbitOn = 1; }
			else { OrbitOn = 0; }
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
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:
void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	// get the proper button bit mask:
	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;
		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;
		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;
		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:
	if( state == GLUT_DOWN ){
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	} 
	else{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse moves while a button is down:
void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );

	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );
		// keep object from turning inside-out or disappearing:
		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 1;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	boxScale = 1;
}


// called when user resizes the window:
void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );
	// don't really need to do anything since the window size is
	// checked each time in Display( ):
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:
void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE ){
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}