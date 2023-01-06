#include "simulation.h"
#include "defs.h"
#include "mesh.h"
#include "stb_image.h"
#include <iostream>
#include <cmath>
#include <paths.h>

const size_t N = 128;

int Simulation::init() {
    gen_terrain();
    plane = Aircraft(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f)), GLIDER);
    plane.rotm = glm::rotate(plane.rotm, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    plane.velocity = glm::inverse(plane.rotm) * glm::vec4(0.0f, 0.0f, 45.0f, 0.0f);
	return 0;
}

void Simulation::gen_terrain() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
	int map_width, map_height, n_channels;
	unsigned char* heightmap = stbi_load(RESOURCES_DIRECTORY"/MountainRangeHeightMap.png", &map_width, &map_height, &n_channels, 0);
	if (!heightmap) {
		printf("Error: loading heightmap\n");
	}
    std::cout << "Height Map: " << map_width << "x" << map_height << "  n_channels: " << n_channels<< std::endl;

    float min_height = -50.0f;
    float max_height = 50.0f;
	float grid_size = 1000.0f;
	float grid_step = map_width/grid_size;
    unsigned int grid_width = grid_size/grid_step;
	int xcnt = 0;
	int zcnt = 0;
	for(float z = -grid_size/2.0f; z < grid_size/2.0f; z += grid_step) {
		for(float x = -grid_size/2.0f; x < grid_size/2.0f; x += grid_step) {
            std::cout << z << std::endl;
			//position
            unsigned int hz = (z+grid_size/2.0f)*map_width;
            unsigned int hx = (x+grid_size/2.0f);
            std::cout << hz << std::endl;
			unsigned char pixel = heightmap[hx + hz];
            float height_scale = pixel/255.0f;
            float height = max_height * height_scale;
            float y = min_height + height;

			glm::vec3 p(x, y, z);
			//texture coord
			glm::vec2 uv((xcnt % 2 == 0) ? 0.0f : 1.0f, (zcnt % 2 == 0) ? 1.0f : 0.0f);
			//normal
			glm::vec3 nrml(0.0f, 1.0f, 0.0f);

			Vertex v = {p, uv, nrml};
			vertices.push_back(v);
			xcnt++;
		}
		zcnt++;
	}
    std::cout << "vertices generated" << std::endl;

#define IX(x, y) ((x) + (y)*grid_width)
	for(int z = 0; z < grid_width-1; z++) {
		for(int x = 0; x < grid_width-1; x++) {
			indices.push_back(IX(x, z));
			indices.push_back(IX(x+1, z+1));
			indices.push_back(IX(x, z+1));

			indices.push_back(IX(x, z));
			indices.push_back(IX(x+1, z));
			indices.push_back(IX(x+1, z+1));
		}
	}

	for(int i = 0; i < indices.size() - 3; i+= 3) {
		Vertex& A = vertices[indices[i]];
		Vertex& B = vertices[indices[i+1]];
		Vertex& C = vertices[indices[i+2]];

		glm::vec3 norm = glm::cross(A.pos - C.pos, A.pos - B.pos);

		//ruins normalized property but tilts direction
		A.normal += norm;
		B.normal += norm;
		C.normal += norm;
	}


}

int Simulation::update(double dt) {
    elapsed += dt;

    glm::vec4 gravity(0.0f, -9.8f, 0.0f, 0.0f);
    glm::vec4 thrust(0.0f, 0.0f, -3000.0f, 0.0f);
    plane.acceleration = glm::inverse(plane.rotm) * gravity;
    plane.thrust = thrust;

    plane.update(dt);
	return 0;
}
