#ifndef OBJECT_H
#define OBJECT_H
#include "mesh.h"
#include <glm/glm.hpp>

class Object {
public:
	Mesh mesh;
	glm::vec3 position;
	float rotation;
	float scale;

	Object();
	void Update(double dt);
};


class Grid : public Object {

};


#endif
