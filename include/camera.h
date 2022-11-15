#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <GL/glew.h>

class Entity;

#define PI 3.141596

class Camera {
public:
    glm::mat4 view;
    glm::mat4 projection;

    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 rotation;

    Entity* target = nullptr;

    float aspect_ratio = 16/9;
    float fov = glm::radians<float>(75.0f);

    float look_sensitivity = 0.25f;
    float zoom_sensitivity = 0.1f;

	Camera() = default;
	Camera(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up, Entity* target);
	void init();
	//void Update(float dt);
	void update();

	void step_yaw(float yaw);
	void step_pitch(float pitch);
	void step_fov(float fov);
    void set_aspect_ratio(float asp);

	void set_look_sensitivity(float sens);
	void set_zoom_sensitivity(float sens);

};
#endif
