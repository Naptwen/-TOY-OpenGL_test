#include "UIMGUI.hpp"
EDITOR sEditor;
KEYBOARD sKeyboard;
MOUSE sMouse;
LIGHT lightA;
IMGUI sImgui;

void activeKeyboard(unsigned char key, int x, int y) {
    if (ImGui::GetIO().WantCaptureKeyboard) {
        ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    }
    else {
        sKeyboard.active(key, x, y, sEditor);
        glutPostRedisplay();
    }
}

void activeMouse(int button, int state, int x, int y) {
    if (ImGui::GetIO().WantCaptureMouse) {
        ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    }
    else {
        sMouse.active(button, state, x, y, sEditor);
        glutPostRedisplay();
    }
}

void activeMotion(int x, int y) {
	sMouse.motion(x, y, sEditor);
	glutPostRedisplay();
}

void activeScroll(int wheel, int direction, int x, int y) {
    sMouse.scroll(wheel, direction, x, y, sEditor);
    glutPostRedisplay();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    sEditor.camera = std::make_shared<CAMERA>();
    sEditor.camera->_viewportWidth = width;
    sEditor.camera->_viewportHeight = height;
    ImGuiIO& io = ImGui::GetIO(); 
    io.DisplaySize = ImVec2((float)width, (float)height);
}

void PERSPECTIVE_VIEW() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    auto projectMatrix = sEditor.camera->getProjectionMatrix();
    glLoadMatrixf(glm::value_ptr(projectMatrix));
}

void CAMERA_VIEW() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    auto viewMatrix = sEditor.camera->getViewMatrix();
    glLoadMatrixf(glm::value_ptr(viewMatrix));
}

void PHYSICS_PIPE() {
    for (size_t i = 0; i < sEditor.models.size(); ++i) {
        sEditor.models[i]->setCollision(false);
    }
    for (size_t i = 0; i < sEditor.models.size(); ++i) {
        for (size_t j = i + 1; j < sEditor.models.size(); ++j) {
            if (sEditor.models[i]->collider == nullptr || sEditor.models[j]->collider == nullptr) {
				continue;
			}
            if (CollisionChecking(*sEditor.models[i]->collider, *sEditor.models[j]->collider)) {
                sEditor.models[i]->setCollision(true);
                sEditor.models[j]->setCollision(true);
			}
        }
    }
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

    int gridSize = 100; 
    float cellSize = 100.0f / gridSize;
    glColor3f(0.5f, 0.5f, 0.5f);
    for (int i = -gridSize / 2; i < gridSize / 2; ++i) {
        for (int j = -gridSize / 2; j < gridSize / 2; ++j) {
            glBegin(GL_LINE_LOOP); // 각 셀을 그립니다.
            glVertex3f(i * cellSize, 0.0f, j * cellSize);
            glVertex3f((i + 1) * cellSize, 0.0f, j * cellSize);
            glVertex3f((i + 1) * cellSize, 0.0f, (j + 1) * cellSize);
            glVertex3f(i * cellSize, 0.0f, (j + 1) * cellSize);
            glEnd();
        }
    }

    glEnable(GL_LIGHTING);
}

void DRAW_COLLIDER() {
    glDisable(GL_LIGHTING);
    for (auto& model : sEditor.models) {
        if (model->collider == nullptr) {
            continue;
        }
        if (model->collider->type == COLLIDER_TYPE::SPHERE) {
            SPHERE_COLLIDER* sphereCollider = dynamic_cast<SPHERE_COLLIDER*>(model->collider.get());
            sphereCollider->DRAW();
        }
        else if (model->collider->type == COLLIDER_TYPE::BOX) {
            BOX_COLLIDER* boxCollider = dynamic_cast<BOX_COLLIDER*>(model->collider.get());
            boxCollider->DRAW();
        }
    }
    glDisable(GL_BLEND);
}

void DRAW_WORLD() {
    lightA.DRAW();
    for(auto& model : sEditor.models) {
		model->DRAW(lightA, *sEditor.camera);
	}
}

void DRAW_GUI() {
    sImgui.begin();
    sImgui.draw(sEditor);
    sImgui.end();
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PERSPECTIVE_VIEW();
    CAMERA_VIEW();
    PHYSICS_PIPE();

    if (sEditor.axisView)
        DRAW_AXIS();
    if (sEditor.colliderView)
        DRAW_COLLIDER();
    DRAW_WORLD();
    DRAW_GUI();

    glutSwapBuffers(); 
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

    int windowWidth = 1204;
    int windowHeight = 624;
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL");
    glewInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::StyleColorsLight();
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init("#version 130");

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

    glutMainLoop();
    return 0;
}
