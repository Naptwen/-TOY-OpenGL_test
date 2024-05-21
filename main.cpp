#include "UGL.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glut.h"
#include "backends/imgui_impl_opengl3.h"

CAMERA camera;
LIGHT lightA;
MODEL model;

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
        case 'W':
            camera.moveForward(1.0f);
            break;
        case 's':
        case 'S':
            camera.moveBackward(1.0f);
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
    camera._viewportWidth = width;
    camera._viewportHeight = height;
    ImGuiIO& io = ImGui::GetIO(); 
    io.DisplaySize = ImVec2((float)width, (float)height);
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

void DRAW_AXIS() {
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
}

void DRAW_WORLD() {
    lightA.Set();
    model.Draw(lightA, camera);
}

void DRAW_GUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize({ io.DisplaySize.x * 0.25f, io.DisplaySize.y});
    ImGui::Begin("GAME MAKING", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    int buttonWidth = 150;
    int buttonHeight = 25;
    if (ImGui::Button("Object Add", ImVec2(buttonWidth, buttonHeight))) {
        printf("Button was pressed.\n");
        glutPostRedisplay();
    }
    if (ImGui::Button("Object Del", ImVec2(buttonWidth, buttonHeight))) {
		printf("Button was pressed.\n");
        glutPostRedisplay();
	}
    if (ImGui::Button("Object Attr", ImVec2(buttonWidth, buttonHeight))) {
        printf("Button was pressed.\n");
        glutPostRedisplay();
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PERSPECTIVE_VIEW();
    CAMERA_VIEW();
    
    DRAW_AXIS();
    DRAW_WORLD();
    DRAW_GUI();

    glutSwapBuffers(); 
}

void imguiInit() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::StyleColorsLight();

    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init("#version 130");
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    int windowWidth = 1204;
    int windowHeight = 624;
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL");

    glewInit();
    imguiInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    reshape(windowWidth, windowHeight);

    model.LoadFBX("D:\\projects\\OpenGL\\OpenGL\\untitled.fbx");

    glutMainLoop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
