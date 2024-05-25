#include "UGL.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glut.h"
#include "backends/imgui_impl_opengl3.h"

CAMERA camera;
EDITOR editor;
KEYBOARD keyboard(&camera);
MOUSE mouse(&camera);
LIGHT lightA;

void activeKeyboard(unsigned char key, int x, int y) {
    if (ImGui::GetIO().WantCaptureKeyboard) {
        ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    }
    else {
        keyboard.active(key, x, y);
        glutPostRedisplay();
    }
}

void activeMouse(int button, int state, int x, int y) {
    if (ImGui::GetIO().WantCaptureMouse) {
        ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    }
    else {
        mouse.active(button, state, x, y, editor);
        glutPostRedisplay();
    }
}

void activeMotion(int x, int y) {
	mouse.motion(x, y);
	glutPostRedisplay();
}

void activeScroll(int wheel, int direction, int x, int y) {
    mouse.scroll(wheel, direction, x, y);
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
    glVertex3f(1000.0, 0.0, 0.0);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1000.0, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1000.0);
    glEnd();
    glEnable(GL_LIGHTING);
}

void DRAW_WORLD() {
    lightA.Set();
    for(auto& model : editor.models) {
		model->Draw(lightA, camera);
	}
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
    }
    if (ImGui::Button("Object Del", ImVec2(buttonWidth, buttonHeight))) {
		printf("Button was pressed.\n");
	}
    if (ImGui::Button("Object Attr", ImVec2(buttonWidth, buttonHeight))) {
        printf("Button was pressed.\n");
    }
    
    if (editor.selectedModel != nullptr)
    {
        ImGui::Text("Selected Model's Properties");
        glm::vec3 pos = editor.selectedModel->getProperty_Position();
        glm::vec3 rot_axis = editor.selectedModel->getProperty_RotationAxis();
        float rot = editor.selectedModel->getProperty_Rotation();
        glm::vec3 scale = editor.selectedModel->getProperty_Scale();

        if (ImGui::InputFloat3("Position", glm::value_ptr(pos))) {
            printf("pos changed\n");
            editor.selectedModel->setProperty_Position(pos);
        }
        if (ImGui::InputFloat3("RotationAxis", glm::value_ptr(rot_axis))) {
            printf("rot_axis changed\n");
            editor.selectedModel->setProperty_RotationAxis(rot_axis);
        }
        if (ImGui::InputFloat("Rotation", &rot)) {
            printf("rot changed\n");
            editor.selectedModel->setProperty_Rotation(rot);
        }
        if (ImGui::InputFloat3("Scale", glm::value_ptr(scale))) {
            printf("scale changed\n");
            editor.selectedModel->setProperty_Scale(scale);
        }
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glutPostRedisplay();
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

    int windowWidth = 1204;
    int windowHeight = 624;
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL");

    glewInit();
    imguiInit();

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(activeKeyboard);
    glutMouseFunc(activeMouse);
    glutMotionFunc(activeMotion);
    glutMouseWheelFunc(activeScroll);

    reshape(windowWidth, windowHeight);

    //model.LoadFBX("D:\\projects\\OpenGL\\OpenGL\\untitled.fbx");

    editor.models.push_back(std::make_unique<CUBE>());
    editor.models.push_back(std::make_unique<CUBE>());
    editor.models.push_back(std::make_unique<FBX>());
    editor.models.at(0)->Init("");
    editor.models.at(1)->Init("");
    std::static_pointer_cast<FBX>(editor.models.at(2))->Init("D:\\projects\\OpenGL\\OpenGL\\test.fbx");

    editor.models.back()->setProperty_Position({ 0.0, 1.0, 0.0 });

    glutMainLoop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
