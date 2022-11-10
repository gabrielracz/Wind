#include<buffer.h>

BufferLayout::BufferLayout(std::initializer_list<BufferElement> elements)
	: _elements(elements) 
{
	for (int i = 0; i < _elements.size(); i++) {
		_stride += _elements[i].GetSize();
	}
}

void BufferLayout::Enable(){

	long offset = 0;
	for (int i = 0; i < _elements.size(); i++) {
		BufferElement e = _elements[i];
		glVertexAttribPointer(i, e.GetCount(), e.GetGLType(), GL_FALSE, _stride, (void*) offset);
		glEnableVertexAttribArray(i);
		offset += e.GetSize();
	}
}

BufferObject::BufferObject(float* vb, uint32_t vb_size, uint32_t* eb, uint32_t eb_size)
{
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(VBO, vb_size, vb, GL_STATIC_DRAW);
	num_vertices = vb_size / sizeof(float);
	if (eb) {
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(EBO, eb_size, eb, GL_STATIC_DRAW);
		eb_defined = true;
		num_indices = eb_size / sizeof(float);
	}
}


VertexArray::VertexArray(BufferObject& buffer, BufferLayout& layout)
	: _buff(buffer), _layout(layout) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	//_buff.Bind();
	_layout.Enable();
	glBindVertexArray(0);
}

void VertexArray::Draw() {
	if (_buff.eb_defined) {
		glDrawElements(GL_TRIANGLES, _buff.GetNumIndices(), GL_UNSIGNED_INT, 0);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}


