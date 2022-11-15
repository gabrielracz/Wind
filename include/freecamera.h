#ifndef FREECAM_H
#define FREECAM_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <GL/glew.h>

#define PI 3.141596
enum MoveDirection {
	FORWARD,
	RIGHT,
	BACK,
	LEFT
};

class FreeCamera {
public:
	FreeCamera();
	FreeCamera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& u);
	void Init();
	//void Update(float dt);
	void Update();
	void Move(MoveDirection);

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

	bool move_forward;
	bool move_back;
	bool move_left;
	bool move_right;

	glm::vec3 position;
	glm::vec3 direction;
//private:
	glm::vec3 up;


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
