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
#include "rope.h"


const int FPS = 60;
bool useIdle = false;

float _k = 5.0f;
float _c = 0.0f;
Eigen::Vector3f start_pos = Eigen::Vector3f(0.0f, 10.0f, 0.0f);
Eigen::Vector3f end_pos = Eigen::Vector3f(20.0f, 10.0f, 0.0f);
int _num_pts = 16;
float _rope_density = 0.06; // Rock climbing rope desity. kg/m
float _unstretched_len = 15.0f;
bool _fixed_tail = false;
float gravity = -9.81f;
float _drag_coef = 0.0f;

Rope rope = Rope(_k, _c, start_pos, end_pos, _num_pts, _rope_density, _unstretched_len, _fixed_tail, gravity, _drag_coef);




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
GLuint	SphereList1;
float	curTime;


////// ##################### FUNCTION PROTOTYPES ##################### //////
// main functions
void	DisplaySetup();
void	Display();
void	Animate();
void	AnimateAtFPS(int);
void	draw_rope(Rope r);
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
	rope.step();
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

	glCallList(SphereList1);

	draw_rope(rope);

	//rope.draw();

	// finish
	glutSwapBuffers();
	glFlush();
  }

void draw_rope(Rope r)
{
	glPushMatrix();
	glEnable(GL_DEPTH_TEST);
	//glColor3f(Colors[c][0], Colors[c][1], Colors[c][2]);
	//glTranslatef(d(0), d(1), d(2));
	//glRotatef(ang, rotAxis(0), rotAxis(1), rotAxis(2));
	//glScalef(scale(0), scale(1), scale(2));
	//init_list(true);
	glLineWidth((GLfloat)5);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < r.num_pts; i++)
	{
		glVertex3f(r.pts[i].pos[0], r.pts[i].pos[1], r.pts[i].pos[2]);
	}
	glEnd();
	glPopMatrix();
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