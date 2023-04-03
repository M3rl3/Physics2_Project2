#pragma once

#include <glm/vec3.hpp>

struct sCamera {
public:
	sCamera() : 
		position(glm::vec3(0.f)), 
		target(glm::vec3(0.f)), 
		forward(glm::vec3(0.f)),
		up(glm::vec3(0.f, 1.f, 0.f)),
		view(glm::mat4(1.f)),
		projection(glm::mat4(1.f)),
		speed(10.f) 
	{};

	~sCamera() {};

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 forward;
	glm::vec3 up;

	glm::mat4 view;
	glm::mat4 projection;

	float speed;

public:
	void MoveUp() 
	{ 
		position += speed * up;
	}
	void MoveDown() 
	{ 
		position += -speed * up;
	}
	void MoveForward() 
	{ 
		position += speed * target; 
	}
	void MoveBackward() 
	{ 
		position += -speed * target; 
	}
	void StrafeLeft() {
		glm::vec3 strafeDirection = glm::cross(target, up);
		position += -speed * strafeDirection;
	}
	void StrafeRight() {
		glm::vec3 strafeDirection = glm::cross(target, up);
		position += speed * strafeDirection;
	}
};