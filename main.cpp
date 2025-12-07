#include <iostream>
#include <GL/freeglut.h>

using namespace std;

// global variables for window width and height
int width = 640;
int height = 480;

// global variables for board drawing parameters
double board_x = 0.5 * (width - height);
double board_y = 0;
double board_size = height;

double square_size = board_size * 0.9 * 0.125;
double square_x = board_x + board_size * 0.05;
double square_y = board_y + board_size * 0.05;

// global variables to keep track of the selected square
int select_x = -1;
int select_y = -1;

// function to update the screen
void draw()
{
	// clear the buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// draw the board background
	glColor3f(0.3, 0.15, 0.1);
	glBegin(GL_QUADS);
	glVertex2f(board_x, board_y);
	glVertex2f(board_x + board_size, board_y);
	glVertex2f(board_x + board_size, board_y + board_size);
	glVertex2f(board_x, board_y + board_size);
	glEnd();

	// draw the board squares
	glBegin(GL_QUADS);
	for (int x = 0; x < 8; ++x)
		for (int y = 0; y < 8; ++y)
		{
			if ((x + y) % 2) glColor3f(0.05, 0.3, 0.05);
			else glColor3f(0.7, 0.65, 0.55);

			glVertex2f(square_x + square_size * x, square_y + square_size * y);
			glVertex2f(square_x + square_size * (x + 1), square_y + square_size * y);
			glVertex2f(square_x + square_size * (x + 1), square_y + square_size * (y + 1));
			glVertex2f(square_x + square_size * x, square_y + square_size * (y + 1));
		}
	glEnd();

	// highlight the selected square
	if (select_x >= 0 && select_y >= 0 && select_x < 8 && select_y < 8)
	{
		glColor3f(0, 1, 0);
		glBegin(GL_QUADS);

		glVertex2f(square_x + square_size * select_x, square_y + square_size * select_y);
		glVertex2f(square_x + square_size * (select_x + 1), square_y + square_size * select_y);
		glVertex2f(square_x + square_size * (select_x + 1), square_y + square_size * (select_y + 1));
		glVertex2f(square_x + square_size * select_x, square_y + square_size * (select_y + 1));

		glEnd();
	}

	// update the display by swapping buffers
	glutSwapBuffers();
}

// function to handle mouse input
void mouse(int button, int state, int x, int y)
{
	// select a square when we click on the window
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		select_x = floor((x - square_x) / square_size);
		select_y = floor((y - square_y) / square_size);
	}
}

// function to handle keyboard input
void keydown(unsigned char key, int x, int y)
{
	if (key == 27) glutLeaveMainLoop(); // exit when the user presses esc
}

// timer function
void timer(int value)
{
	// update the display every 16ms
	glutPostRedisplay();
	glutTimerFunc(16, timer, NULL);
}

// function to handle window resizing
void resize(int w, int h)
{
	// set global width and height
	width = w;
	height = h;

	// recalculate board parameters
	if (width > height)
	{
		board_size = height;
		board_x = 0.5 * (width - height);
		board_y = 0;
	}
	else
	{
		board_size = width;
		board_x = 0;
		board_y = 0.5 * (height - width);
	}

	square_size = board_size * 0.9 * 0.125;
	square_x = board_x + board_size * 0.05;
	square_y = board_y + board_size * 0.05;

	// reset the viewport
	glViewport(0, 0, width, height);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);
}

// main function
int main(int argc, char** argv)
{
	// seed the random number generator
	srand(time(NULL));

	// initialize OpenGL
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	// create a window
	glutInitWindowSize(width, height);
	glutCreateWindow("Chess Tutorial");

	// set the clear color
	glClearColor(0, 0, 0, 1);

	// set up a 2D projection
	gluOrtho2D(0, width, height, 0);

	// register callback functions
	glutDisplayFunc(draw);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keydown);
	glutTimerFunc(16, timer, NULL);
	glutReshapeFunc(resize);

	// enter GLUT's main rendering loop
	glutMainLoop();

	// end program
	return 0;
}