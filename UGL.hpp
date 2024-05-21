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
	glm::vec3 _position = {100.0f, 100.0f, 100.0f};
	glm::vec3 _front = {-1.0f, -1.0f, -1.0f};
	glm::vec3 _up = { 0.0f, 1.0f, 0.0f };
	float _zoom = 1.0f;
	float _near = 0.1f;
	float _far = 1000.0f;
	float _speed = 2.0f;
	int _viewportWidth = 800;
	int _viewportHeight = 600;

	std::tuple<GLdouble, GLdouble, GLdouble, GLdouble> getPerspectiveParameters(){
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
	GLuint VAO, VBO, CBO, NBO, TBO;
	size_t vertexCount;
	unsigned int shaderProgram;

	glm::vec3 _position = { 0.0f, 0.0f, 0.0f }; 
	glm::vec3 _rotationAxis = { 0.0f, 1.0f, 0.0f };
	float _rotation = 0.0f;  
	glm::vec3 _scale = { 1.0f, 1.0f, 1.0f };


	void LoadFBX(const std::string& filename) {
		FbxManager* manager = FbxManager::Create();
		FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
		manager->SetIOSettings(ios);

		FbxImporter* importer = FbxImporter::Create(manager, "");
		if (!importer->Initialize(filename.c_str(), -1, manager->GetIOSettings())) {
		}

		FbxScene* scene = FbxScene::Create(manager, "myScene");
		importer->Import(scene);
		importer->Destroy();

		std::vector<float> vertices;
		std::vector<float> textures;
		std::vector<float> normals;

		FbxNode* rootNode = scene->GetRootNode();
		if (rootNode) {
			for (int i = 0; i < rootNode->GetChildCount(); i++) {
				FbxNode* childNode = rootNode->GetChild(i);
				FbxMesh* mesh = childNode->GetMesh();
				if (mesh) {
					// Process the mesh
					for (int j = 0; j < mesh->GetPolygonCount(); j++) {
						for (int k = 0; k < mesh->GetPolygonSize(j); k++) {
							FbxVector4 vertex = mesh->GetControlPointAt(mesh->GetPolygonVertex(j, k));
							// Process the vertex
							vertices.push_back(static_cast<float>(vertex[0]));
							vertices.push_back(static_cast<float>(vertex[1]));
							vertices.push_back(static_cast<float>(vertex[2]));

							// Process the texture coordinates
							FbxVector2 uv;
							bool unmappedUV;
							FbxStringList uvSetNameList;
							mesh->GetUVSetNames(uvSetNameList);
							mesh->GetPolygonVertexUV(j, k, uvSetNameList.GetStringAt(0), uv, unmappedUV);
							textures.push_back(static_cast<float>(uv[0]));
							textures.push_back(1.0f - static_cast<float>(uv[1]));

							// Process the normals
							FbxVector4 normal;
							mesh->GetPolygonVertexNormal(j, k, normal);
							normals.push_back(static_cast<float>(normal[0]));
							normals.push_back(static_cast<float>(normal[1]));
							normals.push_back(static_cast<float>(normal[2]));
						}
					}
				}
			}
		}
		manager->Destroy();

		std::string vertexShaderSource = ReadShaderFile("VertexShader.vert");
		std::string fragmentShaderSource = ReadShaderFile("FragmentShader.frag");
		shaderProgram = createShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());

		Set(vertices, normals, textures);
	}

	std::string ReadShaderFile(const std::string& filename) {
		std::ifstream _file(filename);
		if (!_file.is_open()) {
			throw std::runtime_error("Failed to open shader file: " + filename);
		}

		std::stringstream _buffer;
		_buffer << _file.rdbuf();

		return _buffer.str();
	}


	std::vector<float> vertices2 = {
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

	std::vector<float> normal2 = {
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

	void Set(const std::vector<float>& vertices,
			const std::vector<float>& normals,
			const std::vector<float>& textures) {
		//cube vertices
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(float), vertices2.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &NBO);
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, normal2.size() * sizeof(float), normal2.data(), GL_STATIC_DRAW);


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

		vertexCount = vertices2.size() / 3;
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
		// 그리기 실행
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
		// 쉐이더 해제
		glUseProgram(currentShader);
	}
};

#endif