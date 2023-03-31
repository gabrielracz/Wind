#include "camera.h"
#include <glm/gtx/string_cast.hpp>
#include <iterator>
#include "aircraft.h"

Camera::Camera() : 
	Camera(	glm::vec3(0.0f, 0.0f, 5.0f),
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec3(0.0f, 1.0f, 0.0f)) {}

Camera::Camera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& u) {
	position = pos;
	original_position = pos;
	direction = front;
	up = u;

	move_forward = false;
	move_back = false;
	move_left = false;
	move_right = false;

	window_width = 800;
	window_height = 450;
	fov = glm::radians<float>(75);
	aspect_ratio = (float)window_width / (float)window_height;


	glm::vec3 xzplane_projection = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
	yaw = acos(glm::dot(xzplane_projection, glm::vec3(1.0f, 0.0f, 0.0f)));
	pitch = 0.0f;
	roll = 0.0f;

	look_sensitivity = 0.25f;
	zoom_sensitivity = 0.1f;
	speed = 0.3f;

	projection = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
	view = glm::lookAt(position, position + direction, up);
    rotation = glm::mat4(1.0f);
}

void Camera::Update() {
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

    projection = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
    if(target != nullptr) {

        glm::vec3 camera_up;
        glm::vec3 eye;
        glm::vec3 center;
        if(locked) {
            camera_up = plane_up;
            eye = target->position + glm::vec3(target->rotm * glm::vec4(position, 0.0f));
            center = target->position;// + glm::vec3(target->rotm * glm::vec4(0.0f, 0.0f, -13.0f, 0.0f));
        } else {
            camera_up = up;
            eye = target->position + glm::vec3(target->rotm * glm::vec4(position, 0.0f));
            center = target->position;// + glm::vec3(target->rotm * glm::vec4(0.0f, 0.0f, -13.0f, 0.0f));
        }

        plane_up = target->rotm * glm::vec4(up, 0.0f);
        view = glm::lookAt(eye, center, camera_up);
    } else {
        view = glm::lookAt(position, position + direction, up);
    }
}

void Camera::Rotate(MoveDirection d) {
    //rotate the position.
    //rotate the up
    float theta = look_sensitivity;

    switch(d) {
        case MoveDirection::UP:
            rotation_amount.x += theta;
            break;
        case MoveDirection::DOWN:
            rotation_amount.x += -theta;
            break;
        case MoveDirection::LEFT:
            rotation_amount.y += theta;
            break;
        case MoveDirection::RIGHT:
            rotation_amount.y += -theta;
            break;
        case MoveDirection::RESET:
            rotation = glm::mat4(1.0f);
            position = original_position;
            rotation_amount = {0,0};
            up = glm::vec3(0.0f, 1.0f, 0.0f);
            Update();
            return;
    }
    rotation = glm::rotate(glm::mat4(1.0f), rotation_amount.x, glm::vec3(1.0f, 0.0f, 0.0f));
    rotation *= glm::rotate(glm::mat4(1.0f), rotation_amount.y, glm::vec3(0.0f, 1.0f, 0.0f));
    position = rotation * glm::vec4(original_position, 1.0f);
    up = rotation * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    Update();
}

void Camera::StepYaw(float y) {
	yaw += y;
}


void Camera::StepPitch(float p) {
	pitch += p;
	if (pitch > PI/2.0f - 0.01f)
		pitch = PI/2.0f - 0.01f;
	else if (pitch < -PI/2.0f + 0.01f)
		pitch = -PI/2.0f + 0.01f;
}

void Camera::StepFov(float f) {
	fov += f*zoom_sensitivity;
	if (fov < PI/180.0f) {
		fov = PI/180.0f;
	}if (fov > PI/2.0f) {
		fov = PI/2.0f;
	}
	projection = glm::perspective(fov, aspect_ratio, 0.1f, 100.0f);
}

void Camera::SetAspectRatio(float asp) {
    aspect_ratio = asp;
}
