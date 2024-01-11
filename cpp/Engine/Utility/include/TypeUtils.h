#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <set>
#include <string>
#include <vector>

#include <stb_image.h>
#include <assimp/color4.h>
#include <assimp/vector3.h>

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

struct MaterialData {
	std::string shader;
	std::vector<std::string> params;
};

struct UniformData {
	glm::mat4x4 modelMatrix;
	glm::mat4x4 viewMatrix;
	glm::mat4x4 projMatrix;
};

struct StateData {
	bool alive;
};

struct MeshData {
	std::string name;
	StateData state;
	UniformData uniform;
	MaterialData material;
	std::vector<uint32_t> indices;
	std::vector<VertexData> vertices;
	std::vector<TextureData> textures;
};
