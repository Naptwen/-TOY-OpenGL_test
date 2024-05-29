#include "UIMGUI.hpp"
EDITOR sEditor;
KEYBOARD sKeyboard;
MOUSE sMouse;
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

void DRAW_GRID() {
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

    for(auto& model : sEditor.models){
        if (model->axisX != nullptr) {
            model->axisX->setStart(model->getProperty_Position());
            model->axisX->DRAW();
        }
        if (model->axisY != nullptr) {
            model->axisY->setStart(model->getProperty_Position());
            model->axisY->DRAW();
        }
        if (model->axisZ != nullptr) {
            model->axisZ->setStart(model->getProperty_Position());
            model->axisZ->DRAW();
        }
	}

    glEnable(GL_LIGHTING);
}

void DRAW_COLLIDER() {
    glDisable(GL_LIGHTING);
    for (auto& light : sEditor.lights) {
		light.DRAW();
	}
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
    for (auto& light : sEditor.lights) {
        light.SET();
    }
    for(auto& model : sEditor.models) {
		model->DRAW(sEditor.lights.at(0), *sEditor.camera);
	}
}

void DRAW_GUI() {
    sImgui.begin();
    sImgui.draw(sEditor);
    sImgui.end();
    glutPostRedisplay();
}

void UPDATE_PHYSICS(){
    for(auto& model : sEditor.models){
        if (model->physics != nullptr) {
            if (model->collider != nullptr && model->collider->collision)
            {
                model->physics->setVelocity(glm::zero<glm::vec3>());
                model->physics->setForce(glm::zero<glm::vec3>());
            }
            glm::vec3 addPos = model->physics->UPDATE(0.1f);
            glm::vec3 pos = model->getProperty_Position();
            model->setProperty_Position(pos + addPos);
        }
	}
}

void engineLoop() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PERSPECTIVE_VIEW();
    CAMERA_VIEW();
    PHYSICS_PIPE();
    if (sEditor.axisView) {
        DRAW_AXIS();
    }
    if (sEditor.gridView) {
		DRAW_GRID();
	}
    if (sEditor.colliderView) {
        DRAW_COLLIDER();
    }
    if (sEditor.modelView) {
        DRAW_GRID();
        DRAW_WORLD();
	}
    DRAW_GUI();
    UPDATE_PHYSICS();

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

    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;

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
    glutDisplayFunc(engineLoop);
    glutKeyboardFunc(activeKeyboard);
    glutMouseFunc(activeMouse);
    glutMotionFunc(activeMotion);
    glutMouseWheelFunc(activeScroll);
    reshape(windowWidth, windowHeight);

    glutMainLoop();
    return 0;
}
