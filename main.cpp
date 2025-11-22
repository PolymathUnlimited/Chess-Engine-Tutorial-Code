#include <iostream>
#include <GL/freeglut.h>

using namespace std;

int width = 640;
int height = 480;

double x_1 = rand() % width;
double x_2 = rand() % width;
double x_3 = rand() % width;
double y_1 = rand() % height;
double y_2 = rand() % height;
double y_3 = rand() % height;

double vx_1 = 0.005 * (rand() % width) - 0.0025;
double vx_2 = 0.005 * (rand() % width) - 0.0025;
double vx_3 = 0.005 * (rand() % width) - 0.0025;
double vy_1 = 0.005 * (rand() % width) - 0.0025;
double vy_2 = 0.005 * (rand() % width) - 0.0025;
double vy_3 = 0.005 * (rand() % width) - 0.0025;

double t = 0;

void draw()
{
	// clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// draw a triangle
	glBegin(GL_TRIANGLES);

	glColor3f(sin(t * 0.9), cos(t * 0.4), 1);	// vertex 1
	glVertex2f(x_1, y_1);

	glColor3f(1, sin(t * 0.8), cos(t * 0.5));	// vertex 2
	glVertex2f(x_2, y_2);

	glColor3f(cos(t * 0.7), 1, sin(t * 0.6));	// vertex 3
	glVertex2f(x_3, y_3);

	glEnd();

	// update the triangle vertex locations
	x_1 += vx_1;
	x_2 += vx_2;
	x_3 += vx_3;
	y_1 += vy_1;
	y_2 += vy_2;
	y_3 += vy_3;

	// reflect off the edges of the window
	if (x_1 < 0 || x_1 >= width) vx_1 *= -1;
	if (x_2 < 0 || x_2 >= width) vx_2 *= -1;
	if (x_3 < 0 || x_3 >= width) vx_3 *= -1;
	if (y_1 < 0 || y_1 >= height) vy_1 *= -1;
	if (y_2 < 0 || y_2 >= height) vy_2 *= -1;
	if (y_3 < 0 || y_3 >= height) vy_3 *= -1;

	// update time (for sin() and cos() in color calculations)
	t += 0.03;

	// end drawing
	glEnd();
	
	// update the display
	glutSwapBuffers();
}

void timer(int value)
{
	// request a display update, then call this function again in 16ms
	glutPostRedisplay();
	glutTimerFunc(16, timer, NULL);
}

int main(int argc, char** argv)
{
	// seed the random number generator
	srand(time(NULL));

	// initialize opengl
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	// create a window
	glutInitWindowSize(width, height);
	glutCreateWindow("Chess Tutorial");

	// set clear color for rendering
	glClearColor(0, 0, 0, 1);

	// set up an orthographic 2d projection
	gluOrtho2D(0, width, height, 0);

	// register callback functions
	glutTimerFunc(16, timer, NULL);
	glutDisplayFunc(draw);

	// enter main rendering loop
	glutMainLoop();

	// end program
	return 0;
}