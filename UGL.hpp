#ifndef __UGL_HPP__
#define __UGL_HPP__
#include "UPHYSIC.hpp"
#include <corecrt_math_defines.h>
#define SELECTION_THRESHOLD 1.0f

inline unsigned int compileShader(unsigned int type, const char* source) {
	unsigned int id = glCreateShader(type);
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

inline unsigned int createShader(const char* vertexShader, const char* fragmentShader, const char* computeShader = nullptr) {
	unsigned int program = glCreateProgram();
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	unsigned int cs;
	if (computeShader) {
		cs = compileShader(GL_COMPUTE_SHADER, computeShader);
		glAttachShader(program, cs);
	}

	glLinkProgram(program);
	int result;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetProgramInfoLog(program, length, &length, message);
		std::cout << "Failed to link program:\n" << message << std::endl;
		glDeleteProgram(program);
		return 0;
	}
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	if (computeShader) {
		glDeleteShader(cs);
	}

	return program;
}

struct CAMERA {
	glm::vec3 _eye = { 30.f, 30.0f, 30.0f };
	glm::vec3 _front = { -1.0f, -1.0f, -1.0f };
	glm::vec3 _up = { 0.0f, 1.0f, 0.0f };
	float _fovy = 15.0f;
	float _near = 0.1f;
	float _far = 10000.0f;
	float _speed = 1.0f;
	float _rot_speed = 0.2f;
	float _yaw = 0.0f;
	float _pitch = 0.0f;
	int _viewportWidth = 800;
	int _viewportHeight = 600;

	void CameraPrintf() const {
		printf("Camera position: (%f, %f, %f)\n",
			_eye.x, _eye.y, _eye.z);
		printf("Camera front: (%f, %f, %f)\n",
			_front.x, _front.y, _front.z);
		printf("Camera vec: (%f, %f, %f)\n",
			_eye.x - _front.x, _eye.y - _front.y, _eye.z - _front.z);
		glm::vec3 vec = glm::normalize(_front - _eye);
		printf("Camera vec : (%f, %f, %f)\n", vec.x, vec.y, vec.z);
	}

	void moveForward(float deltaTime) {
		_eye += _speed * _front * deltaTime;
	}

	void moveBackward(float deltaTime) {
		_eye -= _speed * _front * deltaTime;
	}

	void moveLeft(float deltaTime) {
		_eye -= glm::normalize(glm::cross(_front, _up)) * _speed * deltaTime;
	}

	void moveRight(float deltaTime) {
		_eye += glm::normalize(glm::cross(_front, _up)) * _speed * deltaTime;
	}

	void moveUp(float deltaTime) {
		_eye += _speed * _up * deltaTime;
	}

	void moveDown(float deltaTime) {
		_eye -= _speed * _up * deltaTime;
		CameraPrintf();
	}

	void rotate(float yawOffset, float pitchOffset) {
		yawOffset *= _rot_speed;
		pitchOffset *= _rot_speed;
		printf("Yaw: %f, Pitch: %f\n", yawOffset, pitchOffset);
		// Yaw (Z axis)
		glm::mat4 yawMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(yawOffset), glm::vec3(0.0f, 0.0f, 1.0f));
		// Pitch (Y axis)
		glm::mat4 pitchMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(pitchOffset), glm::vec3(0.0f, 1.0f, 0.0f));
		// Roll (X axis)
		glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 rotationMatrix = yawMatrix * pitchMatrix * rollMatrix;

		glm::vec4 transformedVec = rotationMatrix * glm::vec4(_front, 1.0f);

		_front = glm::vec3(transformedVec);
		CameraPrintf();
	}

	int getViewportWidth() const {
		return _viewportWidth;
	}
	int getViewportHeight()  const {
		return _viewportHeight;
	}
	glm::vec3 getPosition()  const {
		return _eye;
	}
	glm::vec3 getFront() const {
		return _front;
	}
	glm::vec3 getUp() const {
		return _up;
	}
	glm::vec3 getRight() const {
		return glm::normalize(glm::cross(_front, _up));
	}
	glm::mat4 getProjectionMatrix() const {
		float aspect = static_cast<float>(_viewportWidth) / _viewportHeight;
		return glm::perspective(glm::radians(_fovy), aspect, _near, _far);
	}
	glm::mat4 getViewMatrix() const {
		return glm::lookAt(_eye, _eye + _front, _up);
	}
};

struct LIGHT {
	glm::vec3 _pos = { 1.0f, 10.0f, 15.0f };
	glm::vec3 _color = { 1.0f, 1.0f, 1.0f };

	glm::vec3 getPosition() const {
		return _pos;
	}
	glm::vec3 getColor() const {
		return _color;
	}
	void setPosition(glm::vec3 position) {
		_pos = position;
	}
	void setColor(glm::vec3 color) {
		_color = color;
	}

	void SET() {
		GLfloat lightPosition[] = { _pos.x, _pos.y, _pos.z, 1.0f };
		GLfloat lightColor[] = { _color.x, _color.y, _color.z, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	}
	void DRAW() {
		glPushMatrix();
		glTranslatef(_pos.x, _pos.y, _pos.z);
		glColor3f(_color.x, _color.y, _color.z);
		glutWireSphere(2.0f, 10, 10);
		glPopMatrix();
	}
};

struct MODEL_AXIS {
	glm::vec3 color;
	glm::vec3 start;
	glm::vec3 dir;

	glm::vec3 convertMousePosTo3DRay(const glm::vec2& mousePos, CAMERA& camera) {
		// Convert the mouse's 2D position to normalized device coordinates
		glm::vec2 ndcPos;
		ndcPos.x = (2.0f * mousePos.x) / camera._viewportWidth - 1.0f;
		ndcPos.y = 1.0f - (2.0f * mousePos.y) / camera._viewportHeight;

		// Convert from normalized device coordinates to clip coordinates
		glm::vec4 clipPos(ndcPos.x, ndcPos.y, -1.0f, 1.0f);

		// Convert from clip coordinates to eye coordinates
		glm::vec4 eyePos = glm::inverse(camera.getProjectionMatrix()) * clipPos;
		eyePos.z = -1.0f;
		eyePos.w = 0.0f;

		// Convert from eye coordinates to world coordinates
		glm::vec3 worldPos = glm::inverse(camera.getViewMatrix()) * eyePos;

		return glm::normalize(worldPos);
	}

	float distanceBetweenRayAndLine(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& lineStart, const glm::vec3& lineEnd) {
		// Calculate the cross product of the ray direction and the line direction
		glm::vec3 crossProduct = glm::cross(rayDirection, lineEnd - lineStart);

		// Calculate the distance between the ray origin and the line start
		glm::vec3 distanceVector = rayOrigin - lineStart;

		// The distance between the ray and the line is the magnitude of the cross product divided by the magnitude of the line direction
		return glm::length(crossProduct) / glm::length(lineEnd - lineStart);
	}

	glm::vec3 closestPointOnLineToRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& lineStart, const glm::vec3& lineEnd) {
		// Calculate the vector from the ray origin to the line start
		glm::vec3 lineVector = lineStart - rayOrigin;

		// Project the line vector onto the ray direction
		float t = glm::dot(lineVector, rayDirection);

		// Calculate the closest point on the line to the ray
		return rayOrigin + t * rayDirection;
	}

	glm::vec3 getStart() const {
		return start;
	}
	void setStart(const glm::vec3& start) {
		this->start = start;
	}

	void SET(const glm::vec3& color, const glm::vec3& start, const glm::vec3& dir) {
		this->color = color;
		this->start = start;
		this->dir = dir;
	}
	void DRAW() {
		glColor3f(color.x, color.y, color.z);
		glPushMatrix();
		glTranslatef(start.x, start.y, start.z);
		if (dir.x >= 1.0f)
			glScalef(3.0, 0.1f, 0.1f);
		else if (dir.y >= 1.0f)
			glScalef(0.1, 3.0f, 0.1f);
		else if (dir.z >= 1.0f)
			glScalef(0.1, 0.1f, 3.0f);
		glutSolidCube(1.0f); 
		glPopMatrix(); 
	}
	bool isHovered(const glm::vec2& mousePos, CAMERA& camera) {
		glm::vec3 rayOrigin = camera.getPosition();
		glm::vec3 rayDirection = convertMousePosTo3DRay(mousePos, camera);
		float distance = distanceBetweenRayAndLine(rayOrigin, rayDirection, start, start + dir);
		return distance < SELECTION_THRESHOLD;
	}
	glm::vec3 drag(const glm::vec2& mousePos, CAMERA& camera) {
		glm::vec3 rayOrigin = camera.getPosition();
		glm::vec3 rayDirection = convertMousePosTo3DRay(mousePos, camera);
		return closestPointOnLineToRay(rayOrigin, rayDirection, start, start + dir);
	}
};

struct MODEL {

public:
	COLLIDER_TYPE colliderType = COLLIDER_TYPE::NONE;
	std::shared_ptr<COLLIDER> collider;
	std::shared_ptr<PHYSICS> physics;
	std::shared_ptr<MODEL_AXIS> axisX;
	std::shared_ptr<MODEL_AXIS> axisY;
	std::shared_ptr<MODEL_AXIS> axisZ;
	virtual void Init(const std::string&) = 0;

	std::string getName() const {
		return name;
	}
	glm::vec3 getProperty_Position() const {
		return _pos;
	}
	glm::vec3 getProperty_RotationAxis() const {
		return _rotationAxis;
	}
	glm::vec3 getProperty_Scale() const {
		return _scale;
	}
	glm::vec3 getColor() const {
		return _color;
	}

	void setProperty_Position(glm::vec3 position) {
		_pos = position;
		if (collider) {
			collider->position = position;
		}
	}
	void setProperty_RotationAxis(glm::vec3 rotationAxis) {
		_rotationAxis = rotationAxis;
	}
	void setProperty_Scale(glm::vec3 scale) {
		_scale = scale;
	}
	void setColor(glm::vec3 color) {
		_color = color;
	}
	void setName(std::string name) {
		this->name = name;
	}
	void setShaderProgram() {
		std::string vertexShaderSource = ReadShaderFile("VertexShader.vert");
		std::string fragmentShaderSource = ReadShaderFile("FragmentShader.frag");
		//std::string rayShaderSource = ReadShaderFile("ComputeShader.ray");
		shaderProgram = createShader(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
	}
	void setCollision(bool collision) const {
		if (collider != nullptr)
			collider->collision = collision;
	}
	void setCollider() {
		if (colliderType == COLLIDER_TYPE::BOX){
			collider = std::make_shared<BOX_COLLIDER>();
			std::static_pointer_cast<BOX_COLLIDER>(collider)->SET(_pos, _scale, glm::mat3(1.0f));
		}
		else if (colliderType == COLLIDER_TYPE::SPHERE) {
			collider = std::make_shared<SPHERE_COLLIDER>();
			std::static_pointer_cast<SPHERE_COLLIDER>(collider)->SET(_pos);
		}
	}
	void setPhysics(){
		physics = std::make_shared<PHYSICS>();
	}
	void setAxis() {
		axisX = std::make_shared<MODEL_AXIS>();
		axisY = std::make_shared<MODEL_AXIS>();
		axisZ = std::make_shared<MODEL_AXIS>();
		axisX->SET({ 1.0f, 0.0f, 0.0f }, _pos, glm::vec3(3.0f, 0.0f, 0.0f));
		axisY->SET({ 0.0f, 1.0f, 0.0f }, _pos, glm::vec3(0.0f, 3.0f, 0.0f));
		axisZ->SET({ 0.0f, 0.0f, 1.0f }, _pos, glm::vec3(0.0f, 0.0f, 3.0f));
	}

	void SAVE(const char* filename) {
		std::ofstream file(filename);
		file << "Name: " << name << std::endl;
		file << "Color: " << _color.x << " " << _color.y << " " << _color.z << std::endl;
		file << "Position: " << _pos.x << " " << _pos.y << " " << _pos.z << std::endl;
		file << "RotationAxis: " << _rotationAxis.x << " " << _rotationAxis.y << " " << _rotationAxis.z << std::endl;
		file << "Scale: " << _scale.x << " " << _scale.y << " " << _scale.z << std::endl;
		file.close();
	}
	void LOAD(const char* filename) {
		std::ifstream file(filename);
		std::string line;
		while (std::getline(file, line)) {
			std::istringstream iss(line);
			std::string type;
			iss >> type;
			if (type == "Name:") {
				iss >> name;
			}
			else if (type == "Color:") {
				iss >> _color.x >> _color.y >> _color.z;
			}
			else if (type == "Position:") {
				iss >> _pos.x >> _pos.y >> _pos.z;
			}
			else if (type == "RotationAxis:") {
				iss >> _rotationAxis.x >> _rotationAxis.y >> _rotationAxis.z;
			}
			else if (type == "Scale:") {
				iss >> _scale.x >> _scale.y >> _scale.z;
			}
		}
		file.close();
	}

	bool InterSection(glm::vec3 rayOrigin, glm::vec3 rayDirection) const {
		glm::vec3 min = _pos;
		glm::vec3 max = _pos + _scale;
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
	void DRAW(const LIGHT& light, const CAMERA& camera) {
		GLuint VAO = 0, VBO = 0, CBO = 0, NBO = 0, TBO = 0;
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

		size_t vertexCount = vertices.size() / 3;
		// 쉐이더 설정
		glUseProgram(0);
		glUseProgram(shaderProgram);
		// 모델 행렬 설정
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, _pos);
		glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(_rotationAxis[0]), glm::vec3(1, 0, 0));
		glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(_rotationAxis[1]), glm::vec3(0, 1, 0));
		glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(_rotationAxis[2]), glm::vec3(0, 0, 1));
		glm::mat4 rotation = rotationZ * rotationY * rotationX;
		model = model * rotation;
		model = glm::scale(model, _scale);
		int modelLocation = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		// 뷰 행렬 설정
		glm::mat4 view = camera.getViewMatrix();
		int viewLocation = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		// 투영 행렬 설정
		glm::mat4 projection = camera.getProjectionMatrix();
		int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
		// Light position 설정
		int lightPosLocation = glGetUniformLocation(shaderProgram, "lightPos");
		glUniform3fv(lightPosLocation, 1, glm::value_ptr(light._pos));
		// Light color 설정
		int lightColorLocation = glGetUniformLocation(shaderProgram, "lightColor");
		glUniform3fv(lightColorLocation, 1, glm::value_ptr(light._color));
		// Color 설정
		int colorLocation = glGetUniformLocation(shaderProgram, "objectColor");
		glUniform3fv(colorLocation, 1, glm::value_ptr(_color));
		// 그리기 실행
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
		glBindVertexArray(0);
		glUseProgram(0);
		// 메모리 해제
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &NBO);
	}

protected:
	std::string name;
	unsigned int shaderProgram = 0;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> textures;

	glm::vec3 _color = { 1.0f, 1.0f, 1.0f };
	glm::vec3 _pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 _rotationAxis = { 0.0f, 0.0f, 0.0f };
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
};

struct CUBE : public MODEL
{
	void Init(const std::string&) final {
		setShaderProgram();
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

	void LoadFbx(const std::string& filename) {
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
	}

	void Init(const std::string& filename) final {
		name = filename;
		setShaderProgram();
		LoadFbx(filename);
	}
};

#endif