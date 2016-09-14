#include <stdlib.h>
#include <windows.h>
#include <gl/GL.h>
#include <GL/GLU.h>
#include <GL/glut.h>

GLint Width = 512, Height = 512;


void Render() {

}

void ResizeWindow(GLint w, GLint h) {

}

void KeyDown(unsigned char key, int x, int y) {

}


int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(Width, Height);
	glutCreateWindow("OpenGL_APP");

	glutMainLoop();

	glutDisplayFunc(Render);
	glutReshapeFunc(ResizeWindow);
	glutKeyboardFunc(KeyDown);
}