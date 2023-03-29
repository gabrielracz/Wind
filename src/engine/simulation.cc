#include "simulation.h"
#include "defs.h"
#include "mesh.h"
#include "../perlin/perlin.c"
#include "stb_image.h"
#include <iostream>
#include <cmath>
#include <numeric>
#include <paths.h>

const size_t N = 128;

int Simulation::init() {
    gen_terrain();
    plane = Aircraft(glm::vec3(0.0f, 130.0f, 3.0f), glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f)), GLIDER);
    plane.rotm = glm::rotate(plane.rotm, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    plane.velocity = glm::inverse(plane.rotm) * glm::vec4(0.0f, 0.0f, 85.0f, 0.0f);
	return 0;
}

int Simulation::update(double dt) {
    elapsed += dt;

    glm::vec4 gravity(0.0f, -9.8f, 0.0f, 0.0f);
    glm::vec4 thrust(0.0f, 0.0f, -2000.0f, 0.0f);
    plane.acceleration = glm::inverse(plane.rotm) * gravity;
    plane.thrust = thrust;

    plane.update(dt);
	return 0;
}

void Simulation::gen_terrain() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
	int map_width, map_height;
    int n_channels = 1;
	// unsigned char* heightmap = stbi_load(RESOURCES_DIRECTORY"/heightmap.png", &map_width, &map_height, &n_channels, 0);
	// if (!heightmap) {
	// 	printf("Error: loading heightmap\n");
	// }
    // std::cout << "Height Map: " << map_width << "x" << map_height << "  n_channels: " << n_channels<< std::endl;

	perlinit(1337);
    float min_height = -10.0f;
    float max_height = 50.0f;
	float grid_size = 2500.0f;
	// float grid_size = map_width;
	float grid_step = 2.0f;
    unsigned int grid_width = grid_size/grid_step;

	int xcnt = 0;
	int zcnt = 0;
    int pscale = grid_size/2.0f;
	for(float z = -grid_size/2.0f; z < grid_size/2.0f; z += grid_step) {
		for(float x = -grid_size/2.0f; x < grid_size/2.0f; x += grid_step) {
			//position
            // unsigned int hz = (z+grid_size/2.0f + 1)*map_width;
            // unsigned int hx = (x*3+grid_size/2.0f + 1);

            // unsigned char* texel = heightmap + glm::clamp((int)(x + grid_width * z) * n_channels, 0, map_width*map_height);
            // // raw height at coordinate
            // unsigned char y = texel[0];

			// unsigned char pixel = heightmap[hx + hz];
            // float height_scale = pixel/255.0f;
            // std::cout << (int)pixel << std::endl;
            // float height = max_height * height_scale;
            // float y = min_height + height;

            /*                    subsample rate (lower is smoother)     amplitude*/
            const auto ridge = [](float p, float height){
                return abs(p)*-1 + height;
            };
            float perlin_samples[] = {
                perlin((x + pscale)*0.005f, (z + pscale)*0.005f) * 100.0f,
                perlin((x + pscale)*0.01f, (z + pscale)*0.01f) * 100.0f,
                ridge(perlin((x + pscale)*0.025f, (z + pscale)*0.025f) * 30.0f, 30.0f),
                perlin((x + pscale)*0.05f, (z + pscale)*0.05f) * 5.0f,
            };

            float y = 0;
            for(int i = 0; i < sizeof(perlin_samples)/sizeof(float); i++)
                y += perlin_samples[i];

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

#define GIX(x, y) ((x) + (y)*grid_width)
	for(int z = 0; z < grid_width-1; z++) {
		for(int x = 0; x < grid_width-1; x++) {
			indices.push_back(GIX(x, z));
			indices.push_back(GIX(x+1, z+1));
			indices.push_back(GIX(x, z+1));

			indices.push_back(GIX(x, z));
			indices.push_back(GIX(x+1, z));
			indices.push_back(GIX(x+1, z+1));
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
	Layout layout({
		{FLOAT3, "position"},
		{FLOAT2, "uv"},
		{FLOAT3, "normal"}
	});

    Texture t;
    terrain = Mesh(vertices, indices, {t}, layout);

}
