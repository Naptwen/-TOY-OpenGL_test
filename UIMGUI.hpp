#ifndef __UIMGUI_HPP__
#define __UIMGUI_HPP__
#include "imgui.h"
#include "backends/imgui_impl_glut.h"
#include "backends/imgui_impl_opengl3.h"
#include "UGL.hpp"


struct EDITOR {
	std::vector<LIGHT> lights = {LIGHT()};
	std::vector<std::shared_ptr<MODEL>> models;
	std::shared_ptr<MODEL> selectedModel = nullptr;
	std::string currentPath;
	std::shared_ptr<CAMERA> camera;
	bool fileBrowser = false;
	bool colliderView = true;
	bool axisView = true;
	bool gridView = true;
	bool modelView = true;
};

struct KEYBOARD {
	float pressDuration = 0.0f;

	void active(unsigned char key, int x, int y, EDITOR& editor) {
		switch (key) {
		case 'w':
		case 'W':
			editor.camera->moveForward(0.5f);
			break;
		case 's':
		case 'S':
			editor.camera->moveBackward(0.5f);
			break;
		case 'a':
		case 'A':
			editor.camera->moveLeft(0.1f);
			break;
		case 'd':
		case 'D':
			editor.camera->moveRight(0.1f);
			break;
		}
	}
};

struct MOUSE {

	bool _isDragging = false;
	bool _isRotating = false;
	int _lastX = 0;
	int _lastY = 0;
	float pressDuration = 0.0f;
	float sensitivity = 0.1f;

	void active(int button, int state, int x, int y, EDITOR& editor) {
		if (button == GLUT_MIDDLE_BUTTON) {
			if (state == GLUT_DOWN) {
				_isDragging = true;
				_lastX = x;
				_lastY = y;
			}
			else if (state == GLUT_UP) {
				_isDragging = false;
			}
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			if (state == GLUT_DOWN) {
				_isRotating = true;
				_lastX = x;
				_lastY = y;
			}
			else if (state == GLUT_UP) {
				_isRotating = false;
			}
		}
		else if (button == GLUT_LEFT_BUTTON) {
			if (state == GLUT_DOWN) {
				select(x, y, editor);
				for (auto& model : editor.models) {
					if (model != editor.selectedModel)
						continue;
					glm::vec2 mousePos = glm::vec2(x, y);
					if (model->axisX->isHovered(mousePos, *editor.camera)) {
						glm::vec3 pos = model->axisX->drag(mousePos, *editor.camera);
						model->setProperty_Position(pos);
					}
					else if (model->axisY->isHovered(mousePos, *editor.camera)) {
						glm::vec3 pos = model->axisY->drag(mousePos, *editor.camera);
						model->setProperty_Position(pos);
					}
					else if (model->axisZ->isHovered(mousePos, *editor.camera)) {
						glm::vec3 pos = model->axisZ->drag(mousePos, *editor.camera);
						model->setProperty_Position(pos);
					}
				}
			}
		}
	}

	void select(int x, int y, EDITOR& editor) const {
		float ndcX = (2.0f * x) / editor.camera->getViewportWidth() - 1.0f;
		float ndcY = 1.0f - (2.0f * y) / editor.camera->getViewportHeight();

		glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
		glm::vec4 rayEye = glm::inverse(editor.camera->getProjectionMatrix()) * rayClip;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

		glm::vec3 rayOrigin = editor.camera->getPosition();
		glm::vec3 rayDirection = glm::vec3(glm::inverse(editor.camera->getViewMatrix()) * rayEye);
		rayDirection = glm::normalize(rayDirection);

		for (auto& model : editor.models) {
			if (model->InterSection(rayOrigin, rayDirection)) {
				editor.selectedModel = model;
				break;
			}
		}
	}

	void motion(int x, int y, EDITOR& editor) {
		if (_isDragging) {
			float dx = (x - _lastX) * 0.01f;
			float dy = (y - _lastY) * 0.01f;
			editor.camera->moveRight(dx);
			editor.camera->moveUp(dy);
			_lastX = x;
			_lastY = y;
		}
		else if (_isRotating) {
			float xoffset = static_cast<float>((x - _lastX) > 0 ? -1 : 1);
			float yoffset = static_cast<float>((_lastY - y) > 0 ? -1 : 1);
			_lastX = x;
			_lastY = y;
			editor.camera->rotate(yoffset, xoffset);
		}
		else {
			pressDuration = 0.0f;
		}
	}

	void scroll(int wheel, int direction, int x, int y, EDITOR& editor) const {
		if (direction > 0) {
			editor.camera->moveForward(0.5f);
		}
		else {
			editor.camera->moveBackward(0.5f);
		}
	}
};

struct IMGUI {

    float buttonWidth = 150;
    float buttonHeight = 25;

	~IMGUI() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGLUT_Shutdown();
		ImGui::DestroyContext();
	}
	void begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGLUT_NewFrame();
		ImGui::NewFrame();
	}
    void drawFrame() {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize({ io.DisplaySize.x * 0.25f, io.DisplaySize.y });
        ImGui::Begin("GAME MAKING");
    }
    void drawMenu(EDITOR& editor) {
		ImGui::Checkbox("Model View", &editor.modelView);
		ImGui::Checkbox("Collider View", &editor.colliderView);
		ImGui::Checkbox("Grid View", &editor.gridView);
		ImGui::Checkbox("Axis View", &editor.axisView);
    }
	void drawLightProperty(EDITOR& editor) {
		ImGui::Text("Light Properties");
		for (auto& light : editor.lights) {
			glm::vec3 pos = light.getPosition();
			glm::vec3 color = light.getColor();
			if (ImGui::InputFloat3("Light Position", glm::value_ptr(pos))) {
				light.setPosition(pos);
			}
			if (ImGui::InputFloat3("Light Color", glm::value_ptr(color))) {
				light.setColor(color);
			}
		}
	}
	void drawObjectList(EDITOR& editor) {
        if (ImGui::BeginListBox("##models_list"))
        {
            for (auto& model : editor.models)
            {
                std::string modelName = model->getName();
                if (modelName.empty()) {
                    modelName = "##empty_id";
                }
                bool isSelected = (model == editor.selectedModel);
                if (ImGui::Selectable(modelName.c_str(), isSelected))
                {
                    printf("Selected %s\n", modelName.c_str());
                    if (isSelected) {
                        editor.selectedModel = nullptr;
                        model->setColor({ 1.0, 1.0, 1.0 });
                    }
                    else {
                        if (editor.selectedModel != nullptr) {
                            editor.selectedModel->setColor({ 1.0, 1.0, 1.0 });
                        }
                        editor.selectedModel = model;
                        model->setColor({ 1.0, 0.0, 0.0 });
                    }
                    glutPostRedisplay();
                }
            }
            ImGui::EndListBox();
        }
	}
    void drawAddCubeBtn(EDITOR& editor) {
        if (ImGui::Button("Object Cube Add", ImVec2(buttonWidth, buttonHeight))) {
            std::shared_ptr<CUBE> cube = std::make_shared<CUBE>();
            cube->Init("");
			cube->colliderType = COLLIDER_TYPE::BOX;
			cube->setCollider();
			cube->setPhysics();
			cube->setAxis();
            editor.models.push_back(cube);
        }
	}
    void drawAddSphereBtn(EDITOR& editor) {
        if (ImGui::Button("Object Sphere Add", ImVec2(buttonWidth, buttonHeight))) {
            std::shared_ptr<FBX> sphere = std::make_shared<FBX>();
            sphere->Init("D:\\projects\\OpenGL\\OpenGL\\Resource\\sphere.fbx");
            sphere->colliderType = COLLIDER_TYPE::SPHERE;
            sphere->setCollider();
			sphere->setPhysics();
			sphere->setAxis();
            editor.models.push_back(sphere);
        }
    }
    void drawFileExplorer(EDITOR& editor) {
        if (ImGui::Button("File Browser", ImVec2(buttonWidth, buttonHeight))) {
            editor.fileBrowser = !editor.fileBrowser;
        }
        if (editor.fileBrowser)
        {
            editor.currentPath = std::filesystem::current_path().string() + "\\Resource";
            if (ImGui::Begin("File Browser"))
            {
                ImGui::Text("Current Path: %s", editor.currentPath.c_str());
                ImGui::BeginListBox("##file_list");
                for (const auto& entry : std::filesystem::directory_iterator(editor.currentPath))
                {
                    if (entry.path().extension() == ".fbx" && ImGui::Selectable(entry.path().filename().string().c_str()))
                    {
                        std::shared_ptr<FBX> fbx = std::make_shared<FBX>();
                        fbx->Init(entry.path().string());
                        fbx->setCollider();
                        editor.models.push_back(fbx);
                        editor.fileBrowser = false;
                        break;
                    }
                }
                ImGui::EndListBox();
                ImGui::End();
            }
        }
	}
    void drawObjectProperty(EDITOR& editor) {
        if (editor.selectedModel != nullptr)
        {
            ImGui::Text("Selected Model's Properties");
            glm::vec3 pos = editor.selectedModel->getProperty_Position();
            glm::vec3 rot_axis = editor.selectedModel->getProperty_RotationAxis();
            glm::vec3 scale = editor.selectedModel->getProperty_Scale();
			char nameBuffer[256];
			strncpy_s(nameBuffer, sizeof(nameBuffer), editor.selectedModel->getName().c_str(), _TRUNCATE);
			if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
				editor.selectedModel->setName(nameBuffer);
			}
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
			if (editor.selectedModel->collider != nullptr)
			{
				glm::vec3 relative_pos = editor.selectedModel->collider->getRelativePosition();
				glm::vec3 collider_scale = editor.selectedModel->collider->getScale();
				if (ImGui::InputFloat3("Collider Position", glm::value_ptr(relative_pos))) {
					printf("collider pos changed\n");
					editor.selectedModel->collider->setRelativePosition(relative_pos);
				}
				if (ImGui::InputFloat3("Collider Scale", glm::value_ptr(collider_scale))) {
					printf("collider scale changed\n");
					editor.selectedModel->collider->setScale(collider_scale);
				}
			}
			if (editor.selectedModel->physics != nullptr)
			{
				glm::vec3 force = editor.selectedModel->physics->getForce();
				glm::vec3 velocity = editor.selectedModel->physics->getVelocity();
				if (ImGui::InputFloat("Mass", &editor.selectedModel->physics->mass)) {
					printf("Mass changed\n");
					editor.selectedModel->physics->mass = std::max(0.0f, editor.selectedModel->physics->mass);
				}
				if (ImGui::InputFloat3("Force", glm::value_ptr(force))) {
					printf("Force changed\n");
					editor.selectedModel->physics->setForce(force);
				}
				if (ImGui::InputFloat3("Velocity", glm::value_ptr(velocity))) {
					printf("Velocity changed\n");
					editor.selectedModel->physics->setVelocity(velocity);
				}
			}
			if (ImGui::Button("Object Delete", ImVec2(buttonWidth, buttonHeight))) 
			{
				if (editor.selectedModel != nullptr) {
					auto it = std::find(editor.models.begin(), editor.models.end(), editor.selectedModel);
					if (it != editor.models.end()) {
						it->reset();
						editor.models.erase(it);
						editor.selectedModel = nullptr;
					}
				}
			}
        }
	}
	void draw(EDITOR& editor) {
        ImGui::Text("Models:");
        drawMenu(editor);
		drawLightProperty(editor);
        drawObjectList(editor);
        drawAddCubeBtn(editor);
        drawAddSphereBtn(editor);
        drawFileExplorer(editor);
		ImGui::Text("Property");
        drawObjectProperty(editor);
	}
	void end() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};
#endif