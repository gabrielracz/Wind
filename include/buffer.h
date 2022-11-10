#ifndef BUFFER_H
#define BUFFER_H

#include<stdint.h>
#include<stdio.h>
#include<GL/glew.h>
#include<vector>
#include<string>

class BufferObject;
class BufferElement;
class BufferLayout;

enum ShaderDataType {
	Float, Float2, Float3, Float4, Int, Int2, Int3, Int4
};


class BufferElement {
static uint32_t GetShaderDataTypeSize(ShaderDataType type) {
	switch (type) {
	case Float:		return sizeof(float);
	case Float2:	return sizeof(float) * 2;
	case Float3:	return sizeof(float) * 3;
	case Float4:	return sizeof(float) * 4;
	case Int:		return sizeof(int);
	case Int2:		return sizeof(int) * 2;
	case Int3:		return sizeof(int) * 3;
	case Int4:		return sizeof(int) * 4;
	}
}

static uint32_t GetShaderDataTypeCount(ShaderDataType type) {
	switch (type) {
	case Float:		return 1;
	case Float2:	return 2;
	case Float3:	return 3;
	case Float4:	return 4;
	case Int:		return 1;
	case Int2:		return 2;
	case Int3:		return 3;
	case Int4:		return 4;
	}
}

static uint32_t ShaderDataTypeToGLType(ShaderDataType type) {
	switch (type) {
	case Float:		return GL_FLOAT;
	case Float2:	return GL_FLOAT;
	case Float3:	return GL_FLOAT;
	case Float4:	return GL_FLOAT;
	case Int:		return GL_INT;
	case Int2:		return GL_INT;
	case Int3:		return GL_INT;
	case Int4:		return GL_INT;
	}
}

public:
	BufferElement(ShaderDataType type, const std::string& name)
		: _name(name), _type(type), _size(GetShaderDataTypeSize(type)), _count(GetShaderDataTypeCount(type)), _gltype(ShaderDataTypeToGLType(type)) {}
	inline ShaderDataType GetType() { return _type; }
	inline uint32_t GetSize() { return _size; }
	inline uint32_t GetCount() { return _count; }
	inline uint32_t GetGLType() { return _gltype; }

private:
	std::string _name;
	ShaderDataType _type;
	uint32_t _size;
	uint32_t _count;
	uint32_t _gltype;
};


class BufferLayout {
private:
	std::vector<BufferElement> _elements;
	uint32_t _stride;

public:
	BufferLayout() = default;
	BufferLayout(std::initializer_list<BufferElement> elements);
	void Enable();
};

class BufferObject {
private:
	uint32_t VBO;
	uint32_t EBO;
	uint32_t num_indices = 0;
	uint32_t num_vertices;
public:
	bool eb_defined;
public:
	BufferObject(float* vb, uint32_t vb_size, uint32_t* eb = nullptr, uint32_t eb_size = 0) ;

	inline uint32_t GetNumIndices() { return num_indices; }
	inline uint32_t GetNumVertices() { return num_vertices; }

};

class VertexArray {
private:
	uint32_t VAO;
	BufferObject _buff;
	BufferLayout _layout;
public:
	VertexArray(BufferObject& buffer, BufferLayout& layout);
	inline void Bind() { glBindVertexArray(VAO); }
	void Draw();
};

#endif
