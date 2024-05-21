#include "UGL.hpp"

CAMERA camera;
LIGHT lightA;
MODEL model;

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
        case 'W':
            camera.moveForward(0.2);
            break;
        case 's':
        case 'S':
            camera.moveBackward(0.2);
            break;
        case 'a':
        case 'A':
            camera.moveLeft(0.2);
            break;
        case 'd':
        case 'D':
            camera.moveRight(0.2);
            break;
    }
    printf("Camera position: (%f, %f, %f)\n", camera._position.x, camera._position.y, camera._position.z);
    glutPostRedisplay();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void PERSPECTIVE_VIEW() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble _zoom, _aspect, _near, _far;
	std::tie(_zoom, _aspect, _near, _far) = camera.getPerspectiveParameters();
	gluPerspective(_zoom, _aspect, _near, _far);
}

void CAMERA_VIEW() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glm::vec3 eye, center, up;
	std::tie(eye, center, up) = camera.getLookAtParameters();
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PERSPECTIVE_VIEW();
    CAMERA_VIEW();

    lightA.Set();
    model.Draw(lightA, camera);
    // X, Y, Z axis line with R, G, B color
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(10.0, 0.0, 0.0);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 10.0, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 10.0);
    glEnd();
    glEnable(GL_LIGHTING);
    
    glutSwapBuffers(); 
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL");

    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    model.LoadFBX("D:\\projects\\OpenGL\\OpenGL\\untitled.fbx");

    while (true) {
        glutMainLoopEvent();
    }


    return 0;
}
