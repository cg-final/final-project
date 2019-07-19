#include "player.h"

Player::Player(std::string modelName) {
	this->modelName = modelName;
	image = stbi_load("res/images/island.jpg", &width, &height, &nrChannels, 0);
}

Player::Player(Model &model) {
	this->model = model;

	
}

void Player::setModel(std::string modelName) {
	this->modelName = modelName;
}

void Player::viewChange(bool view) {
	this->threeView = view;
}

void Player::move(Movement direction, float dt) {
	float moveSpeed = dt * MoveSpeed;

	if (direction == FORWARD)
		position += front * moveSpeed;
	if (direction == BACKWARD)
		position -= front * moveSpeed;
	if (direction == LEFT)
		position -= right * moveSpeed;
	if (direction == RIGHT)
		position += right * moveSpeed;

	if (position.x < -40.0f)
		position.x = -40.0f;
	if (position.x > 40.0f)
		position.x = 40.0f;
	if (position.z < -40.0f)
		position.z = -40.0f;
	if (position.z > 40.0f)
		position.z = 40.0f;

	int x = round(position.x * 10) + 400;
	int z = round(position.z * 10) + 400;
	float h = image[3 * (z * width + x)];
	floorY = h / 10 - 1;

}


void Player::rotate(GLfloat dpitch, GLfloat dyaw, Camera &camera) {
	yaw += dyaw * MouseSensitivity;
	camera.yaw = yaw;
	camera.pitch += dpitch * MouseSensitivity;

	if (camera.pitch > 89.0f)
		camera.pitch = 89.0f;
	if (camera.pitch < -89.0f)
		camera.pitch = -89.0f;

	camera.updateCameraVectors();
	rotation = 90 - yaw;

	updateCameraVectors();
}

void Player::jump() {
	if (!isFlying) {
		velocityY = 7.0f;
	}
	isFlying = true;
}

void Player::fall(float dt) {
	position.y += (velocityY * dt + G * dt * dt / 2);
	velocityY += G * dt;
	if (position.y <= floorY) {
		position.y = floorY;
		velocityY = 0.0f;
		isFlying = false;
	}
}

void Player::render(Shader &shader, Camera& camera, float dt) {
	shader.use();
	if (isFlying) {
		fall(dt);
	}
	else {
		position.y = floorY;
	}

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.03f, 0.03f, 0.03f));
	shader.SetMatrix4("model", model);
	ResourceManager::GetModel(modelName).Draw(shader);
	if (!threeView) {
		//camera.position = position + 2.0f * glm::normalize(camera.up) - 4.0f * front;
		camera.position = position + glm::vec3(0.0f, 2.0f, 0.0f) - 4.0f * front;
	}
	//this->model.Draw(shader);
}

void Player::updateCameraVectors()
{
	this->front.y = sin(glm::radians(pitch));
	this->front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	this->front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	this->front = glm::normalize(this->front);

	this->right = glm::cross(this->front, this->worldup);
	//this->up = glm::cross(this->right, this->front);
}