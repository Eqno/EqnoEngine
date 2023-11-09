#pragma once

#include <set>
#include <string>
#include <vector>
#include <assimp/color4.h>
#include <assimp/vector3.h>

#include <stb_image.h>

class Vertex;

using Vertexes = std::vector<Vertex>;
using Integers = std::vector<int32_t>;
using UIntegers = std::vector<uint32_t>;

using Strings = std::vector<std::string>;
using CStrings = std::vector<const char*>;
using StringSet = std::set<std::string>;

struct VertexData {
	aiVector3D pos;
	aiColor4D color;
	aiVector3D normal;
	aiVector3D tangent;
	aiVector3D texCoord;
};

struct TextureData {
	int width;
	int height;
	int channels;
	stbi_uc* data;
};

struct MeshData {
	std::string name;
	std::string material;
	std::vector<uint32_t> indices;
	std::vector<VertexData> vertices;
	std::vector<TextureData> textures;
};
