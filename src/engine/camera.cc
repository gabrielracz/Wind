#include "camera.h"
#include <glm/gtx/string_cast.hpp>

FreeCamera::FreeCamera() : 
	FreeCamera(	glm::vec3(0.0f, 0.0f, 5.0f),
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec3(0.0f, 1.0f, 0.0f)) {}

FreeCamera::FreeCamera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& u) {
	position = pos;
	direction = front;
	up = u;

	move_forward = false;
	move_back = false;
	move_left = false;
	move_right = false;

	window_width = 800;
	window_height = 450;
	fov = PI/2.0f;
	aspect_ratio = (float)window_width / (float)window_height;


	glm::vec3 xzplane_projection = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
	yaw = -acos(glm::dot(xzplane_projection, glm::vec3(1.0f, 0.0f, 0.0f)));
	pitch = 0.0f;
	roll = 0.0f;

	look_sensitivity = 0.25f;
	zoom_sensitivity = 0.1f;
	speed = 0.3f;

	projection = glm::perspective(fov, aspect_ratio, 0.1f, 500.0f);
	view = glm::lookAt(position, position + direction, up);
}

void FreeCamera::Update() {
	if (move_forward) {
		position += speed * direction;
	} if (move_back) {
		position -= speed * direction;
	} if (move_left) {
		position -= speed * glm::normalize(glm::cross(direction, up));
	} if (move_right) {
		position += speed * glm::normalize(glm::cross(direction, up));
	}

	direction.x = cos(yaw) * cos(pitch);
	direction.y = sin(pitch);
	direction.z = sin(yaw) * cos(pitch);
    projection = glm::perspective(fov, aspect_ratio, 0.1f, 500.0f);
	view = glm::lookAt(position, position + direction, up);
}

void FreeCamera::Move(MoveDirection d) {
}

void FreeCamera::StepYaw(float y) {
	yaw += y;
}


void FreeCamera::StepPitch(float p) {
	pitch += p;
	if (pitch > PI/2.0f - 0.01f)
		pitch = PI/2.0f - 0.01f;
	else if (pitch < -PI/2.0f + 0.01f)
		pitch = -PI/2.0f + 0.01f;
}

void FreeCamera::StepFov(float f) {
	fov += f*zoom_sensitivity;
	if (fov < PI/180.0f) {
		fov = PI/180.0f;
	}if (fov > PI/2.0f) {
		fov = PI/2.0f;
	}
	projection = glm::perspective(fov, aspect_ratio, 0.1f, 100.0f);
}

void FreeCamera::SetAspectRatio(float asp) {
    aspect_ratio = asp;
}
