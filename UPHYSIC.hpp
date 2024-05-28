#ifndef __UPHYSIC_HPP__
#define __UPHYSIC_HPP__
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

enum class COLLIDER_TYPE {
    NONE,
	SPHERE,
	BOX
};

struct COLLIDER {
    COLLIDER_TYPE type;
    glm::vec3 position;
    glm::vec3 relative_position = glm::zero<glm::vec3>();
    float scale = 1.0f;
    bool collision = false;
    
    glm::vec3 getPosition() const {
		return position;
	}
    glm::vec3 getRelativePosition() const {
		return relative_position;
	}
    float getScale() const {
        return scale;
    }

    void setCollision(bool collision) {
		this->collision = collision;
	}
    void setRelativePosition(glm::vec3 relative_position) {
        this->relative_position = relative_position;
    }
    void setPosition(glm::vec3 position) {
        this->position = position;
    }
    void setScale(float scale) {
        this->scale = scale;
    }

	virtual ~COLLIDER() = default;
    virtual void DRAW() {
        glPushMatrix();
        if (collision)
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        else
            glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        glm::vec3 pos = position + relative_position;
        glTranslatef(pos.x, pos.y, pos.z);
        if (type == COLLIDER_TYPE::SPHERE)
            glutWireSphere(scale, 20, 20);
		else if (type == COLLIDER_TYPE::BOX)
            glutWireCube(scale);
        glPopMatrix();
    }
};

struct SPHERE_COLLIDER : public COLLIDER {

    void SET(glm::vec3 position) {
        this->type = COLLIDER_TYPE::SPHERE;
        this->position = position;
    }
};

struct BOX_COLLIDER : public COLLIDER {
	glm::vec3 size;
    glm::mat3 rotation;

    void SET(glm::vec3 position, glm::vec3 size, glm::mat3 rotation) {
        this->type = COLLIDER_TYPE::BOX;
		this->position = position;
		this->size = size;
		this->rotation = rotation;
	}
};

inline bool CollisionChecking(const COLLIDER& A, const COLLIDER& B) {
    const BOX_COLLIDER* boxA = dynamic_cast<const BOX_COLLIDER*>(&A);
    const BOX_COLLIDER* boxB = dynamic_cast<const BOX_COLLIDER*>(&B);
    const SPHERE_COLLIDER* sphereA = dynamic_cast<const SPHERE_COLLIDER*>(&A);
    const SPHERE_COLLIDER* sphereB = dynamic_cast<const SPHERE_COLLIDER*>(&B);

    if (boxA && boxB) {
        glm::vec3 t = (boxB->position + boxB->relative_position) - (boxA->position + boxA->relative_position);
        glm::mat3 R = glm::transpose(boxA->rotation) * boxB->rotation;
        glm::mat3 AbsR =
            glm::mat3(glm::abs(R[0][0]), glm::abs(R[0][1]), glm::abs(R[0][2]),
                glm::abs(R[1][0]), glm::abs(R[1][1]), glm::abs(R[1][2]),
                glm::abs(R[2][0]), glm::abs(R[2][1]), glm::abs(R[2][2])) + glm::mat3(1e-6f);
        glm::vec3 a = boxA->size * boxA->scale * 0.5f;
        glm::vec3 b = boxB->size * boxB->scale * 0.5f;
        for (int i = 0; i < 3; ++i) {
            float ra = a[i];
            float rb = glm::dot(b, glm::vec3(AbsR[i]));
            if (abs(t[i]) >= ra + rb) return false;
        }
        for (int i = 0; i < 3; ++i) {
            float ra = glm::dot(a, glm::vec3(AbsR[0][i], AbsR[1][i], AbsR[2][i]));
            float rb = b[i];
            if (abs(glm::dot(t, glm::vec3(R[0][i], R[1][i], R[2][i]))) >= ra + rb) return false;
        }
        return true;
    }
    else if (sphereA && sphereB) {
        glm::vec3 t = (sphereB->position + sphereB->relative_position) - (sphereA->position + sphereA->relative_position);
        float r = sphereB->scale + sphereA->scale;
        return glm::dot(t, t) < r * r;
    }
    else if (boxA && sphereB) {
        glm::vec3 t = (sphereB->position + sphereB->relative_position) - (boxA->position + boxA->relative_position);
        glm::vec3 p = glm::clamp(t, -boxA->size * boxA->scale * 0.5f, boxA->size * boxA->scale * 0.5f);
        glm::vec3 q = t - p;
        return glm::dot(q, q) < sphereB->scale * sphereB->scale;
    }
    else if (sphereA && boxB) {
        glm::vec3 t = (boxB->position + boxB->relative_position) - (sphereA->position + sphereA->relative_position);
        glm::vec3 p = glm::clamp(t, -boxB->size * boxB->scale * 0.5f, boxB->size * boxB->scale * 0.5f);
        glm::vec3 q = t - p;
        return glm::dot(q, q) < sphereA->scale * sphereA->scale;
    }

    return false;
}

#endif