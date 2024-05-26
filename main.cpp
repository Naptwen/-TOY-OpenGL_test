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
    auto projectMatrix = camera.getProjectionMatrix();
    glLoadMatrixf(glm::value_ptr(projectMatrix));
}

void CAMERA_VIEW() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    auto viewMatrix = camera.getViewMatrix();
    glLoadMatrixf(glm::value_ptr(viewMatrix));
}

void PHYSICS_PIPE() {
    for (size_t i = 0; i < editor.models.size(); ++i) {
        for (size_t j = i + 1; j < editor.models.size(); ++j) {
            if (editor.models[i]->collider == nullptr || editor.models[j]->collider == nullptr) {
				continue;
			}
            if (CollisionChecking(*editor.models[i]->collider, *editor.models[j]->collider)) {
                editor.models[i]->setColor({ 0.0, 0.5, 1.0 });
                editor.models[j]->setColor({ 0.0, 0.5, 1.0 });
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
    for (auto& model : editor.models) {
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
    for(auto& model : editor.models) {
		model->DRAW(lightA, camera);
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
    float buttonWidth = 150;
    float buttonHeight = 25;
    ImGui::Text("Models:");
    if (ImGui::Button("Collider View", ImVec2(buttonWidth, buttonHeight))) {
		editor.colliderView = !editor.colliderView;
	}
    if (ImGui::BeginListBox("##models_list"))
    {
        for (auto& model : editor.models)
        {
            std::string modelName = model->getName();
            if (modelName.empty()) {
                modelName = "##empty_id";
            }
            if (ImGui::Selectable(modelName.c_str()))
            {
                printf("Selected %s\n", modelName.c_str());
                editor.selectedModel = model;
                model->setColor({ 1.0, 0.0, 0.0 });
                break;
            }
        }
        ImGui::EndListBox();
    }
    ImGui::Text("Property");
    if (ImGui::Button("Object Cube Add", ImVec2(buttonWidth, buttonHeight))) {
        std::shared_ptr<CUBE> cube = std::make_shared<CUBE>();
        cube->Init("");
        editor.models.push_back(cube);
    }
    if (ImGui::Button("Object Sphere Add", ImVec2(buttonWidth, buttonHeight))) {

        std::shared_ptr<FBX> sphere = std::make_shared<FBX>();
        sphere->Init("D:\\projects\\OpenGL\\OpenGL\\Resource\\sphere.fbx");
        sphere->colliderType = COLLIDER_TYPE::SPHERE;
        sphere->setCollider();
        editor.models.push_back(sphere);
    }
    if (editor.selectedModel != nullptr) {

        for (auto& model : editor.models)
        {
			if (model == editor.selectedModel)
                model->setColor({ 1.0, 0.0, 0.0 });
            else
                model->setColor({ 1.0, 1.0, 1.0 });
		}

        if (ImGui::Button("Object Del", ImVec2(buttonWidth, buttonHeight))) {
            auto it = std::find_if(editor.models.begin(), editor.models.end(), 
            [&](const auto& model) {
                return model == editor.selectedModel;
                });
            if (it != editor.models.end()) {
                it->reset();
                editor.models.erase(it);
            }
            editor.selectedModel = nullptr;
        }
    }
    else {
        for (auto& model : editor.models)
        {
            model->setColor({ 1.0, 1.0, 1.0 });
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Object Del", ImVec2(buttonWidth, buttonHeight));
        ImGui::PopStyleVar();
    }
    if (ImGui::Button("Object Attr", ImVec2(buttonWidth, buttonHeight))) {
        printf("Button was pressed.\n");
    }
    if (ImGui::Button("File Browser", ImVec2(buttonWidth, buttonHeight))) {
        editor.fileBrowser = !editor.fileBrowser;
	}
    if (editor.fileBrowser)
    {
        if (editor.currentPath.empty())
        {
            editor.currentPath = std::filesystem::current_path().string() + "\\Resource";
        }
        if (ImGui::Begin("File Browser"))
        {
            ImGui::Text("Current Path: %s", editor.currentPath.c_str());
            if (ImGui::BeginListBox("##file_list"))
            {
                for (const auto& entry : std::filesystem::directory_iterator(editor.currentPath))
                {
                    if (entry.path().extension() == ".fbx")
                    {
                        if (ImGui::Selectable(entry.path().filename().string().c_str()))
                        {
                            if (entry.is_directory())
                            {
                                editor.currentPath = entry.path().string();
                            }
                            else
                            {
                                std::shared_ptr<FBX> fbx = std::make_shared<FBX>();
                                fbx->Init(entry.path().string());
                                fbx->setCollider();
                                editor.models.push_back(fbx);
                                editor.fileBrowser = false;
                                break;
                            }
                        }
                    }
                }
                ImGui::EndListBox();
            }
            ImGui::End();
        }
    }
    if (editor.selectedModel != nullptr)
    {
        ImGui::Text("Selected Model's Properties");
        glm::vec3 pos = editor.selectedModel->getProperty_Position();
        glm::vec3 rot_axis = editor.selectedModel->getProperty_RotationAxis();
        glm::vec3 scale = editor.selectedModel->getProperty_Scale();
        glm::vec3 relative_pos = editor.selectedModel->collider->getRelativePosition();
        float collider_scale = editor.selectedModel->collider->getScale();

        if (ImGui::InputFloat3("Position", glm::value_ptr(pos))) {
            printf("pos changed\n");
            editor.selectedModel->setProperty_Position(pos);
        }
        if (ImGui::InputFloat3("RotationAxis", glm::value_ptr(rot_axis))) {
            printf("rot_axis changed\n");
            editor.selectedModel->setProperty_RotationAxis(rot_axis);
        }
        if (ImGui::InputFloat3("Scale", glm::value_ptr(scale))) {
            printf("scale changed\n");
            editor.selectedModel->setProperty_Scale(scale);
        }
        if (ImGui::InputFloat3("Collider Position", glm::value_ptr(relative_pos))) {
			printf("collider pos changed\n");
            editor.selectedModel->collider->setRelativePosition(relative_pos);
		}
        if (ImGui::InputFloat("Collider Scale", &collider_scale)) {
            printf("collider scale changed\n");
            editor.selectedModel->collider->setScale(collider_scale);
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
    PHYSICS_PIPE();

    DRAW_AXIS();
    if(!editor.colliderView)
        DRAW_COLLIDER();
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

    editor.models.push_back(std::make_shared<CUBE>());
    editor.models.push_back(std::make_shared<CUBE>());
    editor.models.at(0)->Init("");
    editor.models.at(0)->setCollider();
    editor.models.at(1)->Init("");
    editor.models.at(1)->setCollider();
    editor.models.back()->setProperty_Position({ 0.0, 1.0, 0.0 });

    glutMainLoop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
