#include "shapebuilder.h"
//===============UVSPHERE=====================
void ShapeBuilder::GenerateSphere(int segments, float radius, float** vertices, unsigned int** indices, unsigned int* num_vertices, unsigned int* num_indices) {
	*num_vertices = (segments+1) * (segments +1) * 5;
	*vertices = new float[*num_vertices];

	int i = 0;
	for (int y = 0; y <= segments; ++y) {
		for (int x = 0; x <= segments; ++x) {
			float xndc = (float)x / (float)segments;
			float yndc = (float)y / (float)segments;
			unsigned int ix = i * 5;
			//Position
			(*vertices)[ix] = (cos(xndc * TWOPI) * sin(yndc * PI)) * radius;
			(*vertices)[ix + 1] = (cos(yndc * PI)) * radius;
			(*vertices)[ix + 2] = (sin(xndc * TWOPI) * sin(yndc * PI)) * radius;

			//Normal
			//(*vertices)[ix + 3] = 0;
			//(*vertices)[ix + 4] = 0;
			//(*vertices)[ix + 5] = 0;

			//UV
			(*vertices)[ix + 3] = xndc;
			(*vertices)[ix + 4] = yndc;
			i++;
		}
	}

	*num_indices = segments * segments * 6;
	*indices = new unsigned int[*num_indices];
	i = 0;
	for (int y = 0; y < segments; y++) {
		for (int x = 0; x < segments; x++) {
			unsigned int ix = i * 6;
			(*indices)[ix] = (y + 1) * (segments + 1) + x;
			(*indices)[ix + 1] = y * (segments + 1) + x;
			(*indices)[ix + 2] = y * (segments + 1) + x + 1;

			(*indices)[ix + 3] = (y+1) * (segments + 1) + x;
			(*indices)[ix + 4] = y * (segments + 1) + x + 1;
			(*indices)[ix + 5] = (y+1) * (segments + 1) + x + 1;
			i++;
		}
	}

}
