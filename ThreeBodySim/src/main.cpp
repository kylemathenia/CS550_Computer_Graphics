#define _CRT_SECURE_NO_DEPRECATE
// standard
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <chrono>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

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

// src
#include "utils.h"
#include "color.h"
#include "options.h"
#include "Shapes/shapes.h"
#include "glslprogramP5.h"



/* Reduce tail length or frames per second (FPS) if poor performance. */
const int TAIL_LEN = 200;
const int FPS = 60;
bool useIdle = false;


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
GLuint	SphereList1;
int		Light0On;
int		Light1On;
int		Light2On;
int		Frozen;
float	curTime;
GLSLProgram* Pattern;
float	Time;

float Light0Pos[] = { 20., 0., 0. };
float Light1Pos[] = { 0., 20., 0. };
float Light2Pos[] = { 0., -10., 20. };

float Light0Col[] = { Colors[Colors::RED][0], Colors[Colors::RED][1], Colors[Colors::RED][2] };
float Light1Col[] = { Colors[Colors::WHITE][0], Colors[Colors::WHITE][1], Colors[Colors::WHITE][2] };
float Light2Col[] = { Colors[Colors::WHITE][0], Colors[Colors::WHITE][1], Colors[Colors::WHITE][2] };


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
void	DoDistortMenu(int id);
void	DoLight0Key();
void	DoLight1Key();
void	DoLight2Key();
void	DoFreezeKey();
void	showLight0();
void	showLight1();
void	showLight2();
void	drawSphere0();
void	drawSphere1();
void	drawSphere2();
void	showLight0Sphere();
void	showLight1Sphere();
void	showLight2Sphere();


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
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

// Moves things that will be drawn in the display function at a certain rate. 
void
AnimateAtFPS(int)
{
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

	// uniformly scale the scene:
	if (scale < MINSCALE)
		scale = MINSCALE;
	glScalef((GLfloat)scale, (GLfloat)scale * aspectRatio, (GLfloat)scale);
	// rotate the scene:
	glRotatef((GLfloat)rot.y, 0., 1., 0.);
	glRotatef((GLfloat)rot.x, 1., 0., 0.);

	//glPushMatrix();
	//glShadeModel(GL_SMOOTH);
	//glEnable(GL_DEPTH_TEST);
	//glColor3f(Colors[Colors::GREEN][0], Colors[Colors::GREEN][1], Colors[Colors::GREEN][2]);
	//glTranslatef(10 * sin(curTime), 10 * cos(curTime), 10 * sin(0.5f * curTime));
	//glCallList(SphereList1);
	//glPopMatrix();

	////char uTime[] = "uTime";
	//Pattern->Use();
	////Pattern->SetUniformVariable(uTime, Time);
	//glCallList(SphereList1);
	//Pattern->UnUse();	// Pattern->Use(0);  also works



	float uS0 = 0.5;
	char uS0s[] = "uS0";
	float uT0 = 0.5;
	char uT0s[] = "uT0";
	float uD = 0.5;
	char uDs[] = "uD";
	Pattern->Use(); // no more fixed-function – shaders now handle everythingPattern->SetUniformVariable( "uS0", s0);
	Pattern->SetUniformVariable(uT0s, uT0);
	Pattern->SetUniformVariable(uS0s, uS0);
	Pattern->SetUniformVariable(uDs, uD);
	glCallList(SphereList1);
	//OsuSphere();
	Pattern->UnUse(); // go back to fixed-function OpenGL




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

	if (Frozen == 0) { curTime = ((float)glutGet(GLUT_ELAPSED_TIME)) / 1000.f;}
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


	char vert_sh[] = "C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\src\\pattern.vert";
	char frag_sh[] = "C:\\dev\\CS550_Computer_Graphics\\ThreeBodySim\\src\\pattern.frag";
	Pattern = new GLSLProgram();
	bool valid = Pattern->Create(vert_sh, frag_sh);
	if (!valid)
	{
		fprintf(stderr, "Shader cannot be created!\n");
		DoMainMenu(QUIT);
	}
	else
	{
		fprintf(stderr, "Shader created.\n");
	}
	Pattern->SetVerbose(false);

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
	int distortmenu = glutCreateMenu(DoDistortMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);
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
	glutAddSubMenu("View (space)", viewmenu);
	glutAddSubMenu("Tail (t)", tailmenu);
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
	glEnable(GL_NORMALIZE);
	glutSetWindow(mainWindow);
	SphereList1 = getSphereList(3., 30, 30);
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
	else if (c == '0') { DoLight0Key(); }
	else if (c == '1') { DoLight1Key(); }
	else if (c == '2') { DoLight2Key(); }
	else if (c == 'f') { DoFreezeKey(); }
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
	Light0On = 1;
	Light1On = 1;
	Light2On = 1;
	Frozen = 0;
	scale = 0.5f;
	whichProjection = PERSP;
	whichView = (int)Views::CENTER;
	whichTail = (int)Tails::LINES;
	rot.x = rot.y = 0;
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
DoDistortMenu(int id)
{
	//if (id == 0) {sim.b1.distortion = false;}
	//else { sim.b1.distortion = true; }
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
	//sim.reset();
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

void
DoFreezeKey()
{
	if (Frozen == 0) { Frozen = 1; }
	else { Frozen = 0; }
}

void
DoLight0Key()
{
	if (Light0On == 0) { Light0On = 1; }
	else { Light0On = 0; }
}

void
DoLight1Key()
{
	if (Light1On == 0) { Light1On = 1; }
	else { Light1On = 0; }
}

void
DoLight2Key()
{
	if (Light2On == 0) { Light2On = 1; }
	else { Light2On = 0; }
}