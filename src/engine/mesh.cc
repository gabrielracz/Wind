#include "mesh.h"

//Texture::Texture(unsigned int i, std::string t) {
	//id = i;
	//type = t;
//}

LayoutEntry::LayoutEntry(LAYOUT_TYPE t, std::string n) {
	type = t;
	name = n;
}

Layout::Layout(std::initializer_list<LayoutEntry> ents) 
	: entries(ents) {
		for(auto e : entries){
			size += e.size() * e.cnt();
		}
	}

//pass by value to copy the const shape arrays
Mesh::Mesh(std::vector<Vertex> verts, std::vector<unsigned int> inds, std::vector<Texture> texts, Layout lay)
	: indices(inds), textures(texts), layout(lay){
		
		//TODO fix this disgusting shit
	for(auto v : verts) {
		vertices.push_back(v.pos.x);
		vertices.push_back(v.pos.y);
		vertices.push_back(v.pos.z);

		vertices.push_back(v.uv.x);
		vertices.push_back(v.uv.y);

		vertices.push_back(v.normal.x);
		vertices.push_back(v.normal.y);
		vertices.push_back(v.normal.z);
	}
	Setup();
}

//do this copying outside ^^^
Mesh::Mesh(const float* verts, size_t num_verts, const unsigned int* inds, size_t num_inds, const Texture* tex, unsigned int num_tex, Layout lay) {

	vertices.resize(num_verts);
	memcpy(&vertices[0], verts, num_verts*sizeof(float));

	if(inds != nullptr && num_inds > 0) {
		indices.resize(num_inds);
		memcpy(&indices[0], inds, num_inds*sizeof(unsigned int));
	}
	if(tex != nullptr && num_tex > 0) {
		textures.resize(num_tex);
		memcpy(&textures[0], tex, num_tex*sizeof(Texture));
	}

	layout = lay;
	Setup();
}

void Mesh::Setup() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW); 

	if(indices.size() > 0) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	}

	size_t total_size = 0;
	for(auto e : layout.entries) {
		total_size += e.cnt() * e.size();
	}

	size_t offset = 0;
	int i = 0;
	for(LayoutEntry e : layout.entries) {
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, e.cnt(), e.gltype(), GL_FALSE, total_size, (void*)(offset));

		offset += e.size() * e.cnt();
		i++;
	}

	glBindBuffer(GL_VERTEX_ARRAY, 0);
	glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader) {
	//potential check if shader is already in use to avoid call
	shader.use();
	if(textures.size() > 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0].id);
	}

	glBindVertexArray(VAO);
	if(indices.size() > 0) {
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}

	glBindVertexArray(0);

}

size_t LayoutEntry::size() {
	switch(type) {
		case FLOAT1: 
		case FLOAT2:
		case FLOAT3:
		case FLOAT4:
			return sizeof(float);
		case UINT: return sizeof(unsigned int);
		case INT: return sizeof(int);
		default: return 0;
	}
}

unsigned int LayoutEntry::cnt() {
	switch(type) {
		case FLOAT1: return 1;
		case FLOAT2: return 2;
		case FLOAT3: return 3;
		case FLOAT4: return 4;
		case UINT: return sizeof(unsigned int);
		case INT: return sizeof(int);
		default: return 0;
	}
}

unsigned int LayoutEntry::gltype() {
	switch(type) {
		case FLOAT1: 
		case FLOAT2:
		case FLOAT3:
		case FLOAT4:
			return GL_FLOAT;
		case UINT: return GL_UNSIGNED_INT;
		case INT: return GL_INT;
		default: return 0;
	}
}

