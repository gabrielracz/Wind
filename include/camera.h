#ifndef FREECAM_H
#define FREECAM_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <GL/glew.h>

#define PI 3.141596
enum MoveDirection {
	UP,
	DOWN,
	LEFT,
	RIGHT,
    RESET
};

class Aircraft;

class Camera {
public:
	Camera();
	Camera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& u);
	void Init();
	//void Update(float dt);
	void Update();
    void Rotate(MoveDirection);

	void SetFov(float fov);
	void SetYaw(float yaw);
	void SetPitch(float pitch);
	void SetRoll(float roll);
	void StepYaw(float yaw);
	void StepPitch(float pitch);
	void StepFov(float fov);
    void SetAspectRatio(float asp);

	void SetLookSens(float sens);
	void SetZoomSense(float sens);

	glm::mat4 view;
	glm::mat4 projection;
    glm::mat4 rotation;
    glm::vec2 rotation_amount = {0, 0};

	bool move_forward;
	bool move_back;
	bool move_left;
	bool move_right;

	glm::vec3 position;
    glm::vec3 original_position;
	glm::vec3 direction;
//private:
	glm::vec3 up;

    Aircraft* target = nullptr;


	int window_width;
	int window_height;
	float aspect_ratio;
	float fov;

	float speed;
	float look_sensitivity;
	float zoom_sensitivity;

	float yaw;
	float pitch;
	float roll;
};
#endif
