#include "model.h"
#include <stdlib.h>
#include <iostream>

const size_t N = 128;

Model::Model() {
}

int Model::Init() {
	grid.data = (float*) malloc(sizeof(float) * N * N * N);
	grid.size = N * N * N;
	grid.w = N;
	grid.h = N;
	grid.d = N;

	for(int i = 0; i < grid.size; i++) {
		grid.data[i] = ((float)rand() / (float)RAND_MAX) * 1.0f;
	}

	return 0;
}

int Model::Step(double dt) {
	//for(int i = 0; i < grid.size; i++) {
		//grid.data[i] = ((float)rand() / (float)RAND_MAX) * 1.0f;
	//}
	return 0;
}
