#ifndef MODEL_H
#define MODEL_H
#include<stdlib.h>

typedef struct {
	float* data;
	size_t size;
	int w,h,d;
}Grid;

class Model {
public:
	Grid grid;
	Model();
	int Init();
	int Step(double dt);
};

#endif
