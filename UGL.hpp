#ifndef __UGL_HPP__
#define __UGL_HPP__
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <tuple>
#include <fstream>
#include <sstream>
#include <fbxsdk.h>
#include <filesystem>



unsigned int compileShader(unsigned int type, const char* source) {
	unsigned int id = glCreateShader(type);
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

unsigned int createShader(const char* vertexShader, const char* fragmentShader) {
	unsigned int program = glCreateProgram();
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

struct CAMERA {
	glm::vec3 _position = { 100.0f, 100.0f, 100.0f };
	glm::vec3 _front = { -1.0f, -1.0f, -1.0f };
	glm::vec3 _up = { 0.0f, 1.0f, 0.0f };
	float _zoom = 1.0f;
	float _near = 0.1f;
	float _far = 10000.0f;
	float _speed = 2.0f;
	int _viewportWidth = 800;
	int _viewportHeight = 600;

	std::tuple<GLdouble, GLdouble, GLdouble, GLdouble> getPerspectiveParameters() {
		return std::make_tuple(_zoom, static_cast<GLdouble>(_viewportWidth) / _viewportHeight, _near, _far);
	}

	std::tuple<glm::vec3, glm::vec3, glm::vec3> getLookAtParameters() {
		glm::vec3 lookAt = _position + _front;
		return std::make_tuple(_position, lookAt, _up);
	}

	void moveForward(float deltaTime) {
		_position += _speed * _front * deltaTime;
	}

	void moveBackward(float deltaTime) {
		_position -= _speed * _front * deltaTime;
	}

	void moveLeft(float deltaTime) {
		_position -= glm::normalize(glm::cross(_front, _up)) * _speed * deltaTime;
	}

	void moveRight(float deltaTime) {
		_position += glm::normalize(glm::cross(_front, _up)) * _speed * deltaTime;
	}

	void moveUp(float deltaTime) {
		_position += _speed * _up * deltaTime;
	}

	void moveDown(float deltaTime) {
		_position -= _speed * _up * deltaTime;
	}

	int getViewportWidth() {
		return _viewportWidth;
	}
	int getViewportHeight() {
		return _viewportHeight;
	}
	glm::vec3 getPosition() {
		return _position;
	}
	glm::vec3 getFront() {
		return _front;
	}
	glm::mat4 getProjectionMatrix() const {
		return glm::perspective(glm::radians(_zoom), static_cast<float>(_viewportWidth) / _viewportHeight, _near, _far);
	}
	glm::mat4 getViewMatrix() const {
		return glm::lookAt(_position, _position + _front, _up);
	}
};

struct LIGHT {
	glm::vec3 _position = { 1.0f, 10.0f, 15.0f };
	glm::vec3 _color = { 1.0f, 1.0f, 1.0f };

	void Set() {
		GLfloat lightPosition[] = { _position.x, _position.y, _position.z, 1.0f };
		GLfloat lightColor[] = { _color.x, _color.y, _color.z, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	}
};

struct MODEL {

public:

	virtual void Init(const std::string&) = 0;
	glm::vec3 getProperty_Position() {
		return _position;
	}
	void setProperty_Position(glm::vec3 position) {
		_position = position;
	}
	glm::vec3 getProperty_RotationAxis() {
		return _rotationAxis;
	}
	void setProperty_RotationAxis(glm::vec3 rotationAxis) {
		_rotationAxis = rotationAxis;
	}
	float getProperty_Rotation() {
		return _rotation;
	}
	void setProperty_Rotation(float rotation) {
		_rotation = rotation;
	}
	glm::vec3 getProperty_Scale() {
		return _scale;
	}
	void setProperty_Scale(glm::vec3 scale) {
		_scale = scale;
	}
	glm::vec3 getColor(){
		return _color;
	}
	void setColor(glm::vec3 color) {
		_color = color;
	}

	bool InterSection(glm::vec3 rayOrigin, glm::vec3 rayDirection) {
		glm::vec3 min = _position;
		glm::vec3 max = _position + _scale;
		float tmin = (min.x - rayOrigin.x) / rayDirection.x;
		float tmax = (max.x - rayOrigin.x) / rayDirection.x;

		if (tmin > tmax) std::swap(tmin, tmax);

		float tymin = (min.y - rayOrigin.y) / rayDirection.y;
		float tymax = (max.y - rayOrigin.y) / rayDirection.y;

		if (tymin > tymax) std::swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax)) return false;

		if (tymin > tmin) tmin = tymin;
		if (tymax < tmax) tmax = tymax;

		float tzmin = (min.z - rayOrigin.z) / rayDirection.z;
		float tzmax = (max.z - rayOrigin.z) / rayDirection.z;

		if (tzmin > tzmax) std::swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax)) return false;

		if (tzmin > tmin) tmin = tzmin;
		if (tzmax < tmax) tmax = tzmax;

		return tmax >= 0;
	}

	void Draw(const LIGHT& light, const CAMERA& camera) const {
		GLint currentShader;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);
		// 쉐이더 설정
		glUseProgram(shaderProgram);
		// 모델 행렬 설정
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, _position);
		model = glm::rotate(model, glm::radians(_rotation), _rotationAxis);
		model = glm::scale(model, _scale);
		int modelLocation = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		// 뷰 행렬 설정
		glm::mat4 view = glm::lookAt(camera._position, camera._position + camera._front, camera._up);
		int viewLocation = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		// 투영 행렬 설정
		glm::mat4 projection = glm::perspective(glm::radians(camera._zoom), (float)camera._viewportWidth / (float)camera._viewportHeight, camera._near, camera._far);
		int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
		// Light position 설정
		int lightPosLocation = glGetUniformLocation(shaderProgram, "lightPos");
		glUniform3fv(lightPosLocation, 1, glm::value_ptr(light._position));
		// Light color 설정
		int lightColorLocation = glGetUniformLocation(shaderProgram, "lightColor");
		glUniform3fv(lightColorLocation, 1, glm::value_ptr(light._color));
		// Color 설정
		int colorLocation = glGetUniformLocation(shaderProgram, "objectColor");
		glUniform3fv(colorLocation, 1, glm::value_ptr(_color));
		// 그리기 실행
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
		// 쉐이더 해제
		glUseProgram(currentShader);
	}

protected:
	std::string name;
	GLuint VAO, VBO, CBO, NBO, TBO;
	size_t vertexCount;
	unsigned int shaderProgram;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> textures;

	glm::vec3 _color = { 1.0f, 1.0f, 1.0f };
	glm::vec3 _position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 _rotationAxis = { 0.0f, 1.0f, 0.0f };
	float _rotation = 0.0f;
	glm::vec3 _scale = { 1.0f, 1.0f, 1.0f };

	std::string ReadShaderFile(const std::string& filename) {
		std::ifstream _file(filename);
		if (!_file.is_open()) {
			throw std::runtime_error("Failed to open shader file: " + filename);
		}

		std::stringstream _buffer;
		_buffer << _file.rdbuf();

		return _buffer.str();
	}
	void Set() {
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &NBO);
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		// vertices:
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);

		//// normal:
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);

		vertexCount = vertices.size() / 3;
	}
};

struct KEYBOARD {
	CAMERA* camera;
	KEYBOARD(CAMERA* camera) : camera(camera) {}

	void active(unsigned char key, int x, int y) {
		switch (key) {
		case 'w':
		case 'W':
			camera->moveForward(2.0f);
			break;
		case 's':
		case 'S':
			camera->moveBackward(2.0f);
			break;
		case 'a':
		case 'A':
			camera->moveLeft(0.5f);
			break;
		case 'd':
		case 'D':
			camera->moveRight(0.5f);
			break;
		}
		printf("Camera position: (%f, %f, %f)\n", camera->_position.x, camera->_position.y, camera->_position.z);
	}
};

struct EDITOR {
	std::vector<std::shared_ptr<MODEL>> models;
	std::shared_ptr<MODEL> selectedModel = nullptr;
};

struct MOUSE {
	std::shared_ptr<CAMERA> camera;
	bool _isDragging = false;
	int _lastX = 0;
	int _lastY = 0;

	MOUSE(CAMERA* camera) : camera(camera) {}

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
		else if (button == GLUT_LEFT_BUTTON) {
			if (state == GLUT_DOWN) {
				select(x, y, editor);
			}
		}
	}

	void select(int x, int y, EDITOR& editor) {
		float ndcX = (2.0f * x) / camera->getViewportWidth() - 1.0f;
		float ndcY = 1.0f - (2.0f * y) / camera->getViewportHeight();

		glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
		glm::vec4 rayEye = glm::inverse(camera->getProjectionMatrix()) * rayClip;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

		glm::vec3 rayOrigin = camera->getPosition();
		glm::vec3 rayDirection = glm::vec3(glm::inverse(camera->getViewMatrix()) * rayEye);
		rayDirection = glm::normalize(rayDirection);

		editor.selectedModel = nullptr;

		for (auto& model : editor.models) {
			model->setColor({ 1.0f, 1.0f, 1.0f });
		}

		for (auto& model : editor.models) {
			if (model->InterSection(rayOrigin, rayDirection)) {
				editor.selectedModel = model;
				editor.selectedModel->setColor({ 1.0f, 0.0f, 0.0f });
				break;
			}
		}

	}

	void motion(int x, int y) {
		if (_isDragging) {
			float dx = (x - _lastX) * 0.01f;
			float dy = (y - _lastY) * 0.01f;
			camera->moveRight(dx);
			camera->moveUp(dy);
			_lastX = x;
			_lastY = y;
		}
	}
	
	void scroll(int wheel, int direction, int x, int y) {
		if (direction > 0) {
			camera->moveForward(2.0f);
		}
		else {
			camera->moveBackward(2.0f);
		}
	}
};

struct CUBE : public MODEL
{
	void Init(const std::string&) final {
		vertices = {
			// Front face
			-0.5f, -0.5f,  0.5f,  // Bottom-left
			 0.5f, -0.5f,  0.5f,  // Bottom-right
			 0.5f,  0.5f,  0.5f,  // Top-right
			 0.5f,  0.5f,  0.5f,  // Top-right
			-0.5f,  0.5f,  0.5f,  // Top-left
			-0.5f, -0.5f,  0.5f,  // Bottom-left

			// Back face
			-0.5f, -0.5f, -0.5f,  // Bottom-left
			-0.5f,  0.5f, -0.5f,  // Top-left
			 0.5f,  0.5f, -0.5f,  // Top-right
			 0.5f,  0.5f, -0.5f,  // Top-right
			 0.5f, -0.5f, -0.5f,  // Bottom-right
			-0.5f, -0.5f, -0.5f,  // Bottom-left

			// Left face
			-0.5f,  0.5f,  0.5f,  // Top-right
			-0.5f,  0.5f, -0.5f,  // Top-left
			-0.5f, -0.5f, -0.5f,  // Bottom-left
			-0.5f, -0.5f, -0.5f,  // Bottom-left
			-0.5f, -0.5f,  0.5f,  // Bottom-right
			-0.5f,  0.5f,  0.5f,  // Top-right

			// Right face
			 0.5f,  0.5f,  0.5f,  // Top-left
			 0.5f, -0.5f,  0.5f,  // Bottom-left
			 0.5f, -0.5f, -0.5f,  // Bottom-right
			 0.5f, -0.5f, -0.5f,  // Bottom-right
			 0.5f,  0.5f, -0.5f,  // Top-right
			 0.5f,  0.5f,  0.5f,  // Top-left

			 // Top face
			 -0.5f,  0.5f, -0.5f,  // Top-left
			 -0.5f,  0.5f,  0.5f,  // Bottom-left
			  0.5f,  0.5f,  0.5f,  // Bottom-right
			  0.5f,  0.5f,  0.5f,  // Bottom-right
			  0.5f,  0.5f, -0.5f,  // Top-right
			 -0.5f,  0.5f, -0.5f,  // Top-left

			 // Bottom face
			 -0.5f, -0.5f, -0.5f,  // Top-right
			  0.5f, -0.5f, -0.5f,  // Top-left
			  0.5f, -0.5f,  0.5f,  // Bottom-left
			  0.5f, -0.5f,  0.5f,  // Bottom-left
			 -0.5f, -0.5f,  0.5f,  // Bottom-right
			 -0.5f, -0.5f, -0.5f   // Top-right
		};
		normals = {
			// Front face normals (0.0, 0.0, 1.0)
			0.0f,  0.0f,  1.0f,  // Bottom-left
			0.0f,  0.0f,  1.0f,  // Bottom-right
			0.0f,  0.0f,  1.0f,  // Top-right
			0.0f,  0.0f,  1.0f,  // Top-right
			0.0f,  0.0f,  1.0f,  // Top-left
			0.0f,  0.0f,  1.0f,  // Bottom-left

			// Back face normals (0.0, 0.0, -1.0)
			0.0f,  0.0f, -1.0f,  // Bottom-left
			0.0f,  0.0f, -1.0f,  // Top-left
			0.0f,  0.0f, -1.0f,  // Top-right
			0.0f,  0.0f, -1.0f,  // Top-right
			0.0f,  0.0f, -1.0f,  // Bottom-right
			0.0f,  0.0f, -1.0f,  // Bottom-left

			// Left face normals (-1.0, 0.0, 0.0)
			-1.0f,  0.0f,  0.0f,  // Top-right
			-1.0f,  0.0f,  0.0f,  // Top-left
			-1.0f,  0.0f,  0.0f,  // Bottom-left
			-1.0f,  0.0f,  0.0f,  // Bottom-left
			-1.0f,  0.0f,  0.0f,  // Bottom-right
			-1.0f,  0.0f,  0.0f,  // Top-right

			// Right face normals (1.0, 0.0, 0.0)
			1.0f,  0.0f,  0.0f,  // Top-left
			1.0f,  0.0f,  0.0f,  // Bottom-left
			1.0f,  0.0f,  0.0f,  // Bottom-right
			1.0f,  0.0f,  0.0f,  // Bottom-right
			1.0f,  0.0f,  0.0f,  // Top-right
			1.0f,  0.0f,  0.0f,  // Top-left

			// Top face normals (0.0, 1.0, 0.0)
			0.0f,  1.0f,  0.0f,  // Top-left
			0.0f,  1.0f,  0.0f,  // Bottom-left
			0.0f,  1.0f,  0.0f,  // Bottom-right
			0.0f,  1.0f,  0.0f,  // Bottom-right
			0.0f,  1.0f,  0.0f,  // Top-right
			0.0f,  1.0f,  0.0f,  // Top-left

			// Bottom face normals (0.0, -1.0, 0.0)
			0.0f, -1.0f,  0.0f,  // Top-right
			0.0f, -1.0f,  0.0f,  // Top-left
			0.0f, -1.0f,  0.0f,  // Bottom-left
			0.0f, -1.0f,  0.0f,  // Bottom-left
			0.0f, -1.0f,  0.0f,  // Bottom-right
			0.0f, -1.0f,  0.0f   // Top-right
		};
		name = "Cube";

		std::string vertexShaderSource = ReadShaderFile("VertexShader.vert");
		std::string fragmentShaderSource = ReadShaderFile("FragmentShader.frag");
		shaderProgram = createShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
		Set();
	}
};

struct FBX : public MODEL
{
	void ProcessMesh(FbxMesh* mesh) {
		if (!mesh) return;

		FbxStringList uvSetNameList;
		mesh->GetUVSetNames(uvSetNameList);
		const char* uvSetName = uvSetNameList.GetCount() > 0 ? uvSetNameList.GetStringAt(0) : nullptr;

		int polygonCount = mesh->GetPolygonCount();
		for (int i = 0; i < polygonCount; i++) {
			int polygonSize = mesh->GetPolygonSize(i);

			// Ensure the polygon is triangulated
			if (polygonSize != 3) {
				std::cerr << "Non-triangular polygon detected. This example only supports triangular polygons." << std::endl;
				continue;
			}

			for (int j = 0; j < polygonSize; j++) {
				int controlPointIndex = mesh->GetPolygonVertex(i, j);

				// Process the vertex
				FbxVector4 vertex = mesh->GetControlPointAt(controlPointIndex);
				vertices.push_back(static_cast<float>(vertex[0]));
				vertices.push_back(static_cast<float>(vertex[1]));
				vertices.push_back(static_cast<float>(vertex[2]));

				// Process the normal
				FbxVector4 normal;
				mesh->GetPolygonVertexNormal(i, j, normal);
				normals.push_back(static_cast<float>(normal[0]));
				normals.push_back(static_cast<float>(normal[1]));
				normals.push_back(static_cast<float>(normal[2]));

				// Process the texture coordinates
				if (uvSetName) {
					FbxVector2 uv;
					bool unmappedUV;
					if (mesh->GetPolygonVertexUV(i, j, uvSetName, uv, unmappedUV)) {
						textures.push_back(static_cast<float>(uv[0]));
						textures.push_back(1.0f - static_cast<float>(uv[1])); // Invert Y-axis for UV coordinates
					}
					else {
						textures.push_back(0.0f);
						textures.push_back(0.0f);
					}
				}
				else {
					textures.push_back(0.0f);
					textures.push_back(0.0f);
				}
			}
		}
	}

	void ProcessNode(FbxNode* node) {
		if (!node) return;

		FbxMesh* mesh = node->GetMesh();
		if (mesh) {
			FbxGeometryConverter geometryConverter(node->GetFbxManager());
			FbxNodeAttribute* newAttribute = geometryConverter.Triangulate(mesh, true);
			if (newAttribute && newAttribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
				FbxMesh* triangulatedMesh = static_cast<FbxMesh*>(newAttribute);
				ProcessMesh(triangulatedMesh);
			}
			else {
				std::cerr << "Failed to triangulate mesh." << std::endl;
			}
		}

		for (int i = 0; i < node->GetChildCount(); i++) {
			ProcessNode(node->GetChild(i));
		}
	}

	void Init(const std::string& filename) final {
		FbxManager* manager = FbxManager::Create();
		FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
		manager->SetIOSettings(ios);

		FbxImporter* importer = FbxImporter::Create(manager, "");
		if (!importer->Initialize(filename.c_str(), -1, manager->GetIOSettings())) {
			std::cerr << "Failed to initialize importer." << std::endl;
			return;
		}

		FbxScene* scene = FbxScene::Create(manager, "myScene");
		importer->Import(scene);
		importer->Destroy();

		FbxNode* rootNode = scene->GetRootNode();
		if (rootNode) {
			ProcessNode(rootNode);
		}

		manager->Destroy();

		std::string vertexShaderSource = ReadShaderFile("VertexShader.vert");
		std::string fragmentShaderSource = ReadShaderFile("FragmentShader.frag");
		shaderProgram = createShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());

		Set();
	}
};

#endif