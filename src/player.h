#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <string>
#include "model.h"
#include "camera.h"
#include "resourceManager/resource_manager.h"

class Player {
public:
	Player() {};
	Player(std::string modelName);
	Player(Model &model);
	~Player() {};
	void setModel(std::string modelName);
	void render(Shader&, Camera& camera, float dt);
	void move(Movement direction, float dt);
	void rotate(GLfloat dpitch, GLfloat dyaw, Camera&);
	void viewChange(bool);
	void jump();
	void fall(float);

public:
	glm::vec3	position = glm::vec3(20.0f, 1.8f, 0.0f);
	GLfloat     pitch = 0, yaw;
	glm::vec3	worldup = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3	front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3	right = glm::vec3(1.0f, 0.0f, 0.0f);

private:
	Model model;
	std::string modelName;
	float rotation = 180;
	bool threeView = true;
	unsigned char* image;
	int width, height, nrChannels;
	float floorY = 1.8f;

	const float G = -9.8f;
	float velocityY = 0.0f;
	bool isFlying = false;

	const float MouseSensitivity = 0.1f;
	const float MoveSpeed = 10.0f;

	void updateCameraVectors();
};


#endif