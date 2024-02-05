#include "../include/draw.h"

#include <vector>

#include "../include/config.h"

void Draw::CreateDrawResource(const Device& device, const std::string& rootPath,
                              const std::vector<std::string>& shaderPaths,
                              const VkRenderPass& renderPass,
                              const int texCount) {
  shader.AddDefinitions({{"MaxLightNum", std::to_string(MaxLightNum)}});
  std::vector<std::string> shaderSearchPaths;
  for (const std::string& searchPath : ShaderSearchPaths) {
    shaderSearchPaths.push_back(rootPath + searchPath);
  }
  shader.SetFileIncluder(shaderSearchPaths);
  shader.SetOptimizationLevel(ShaderOptimizationLevel);
  pipeline.CreatePipeline(device, shader, rootPath, shaderPaths, renderPass,
                          texCount);
}

void Draw::LoadDrawResource(const Device& device, const Render& render,
                            std::weak_ptr<MeshData> data) {
  Mesh* mesh =
      Create<Mesh>(device, render, data, pipeline.GetDescriptorSetLayout());
  meshes.emplace_back(mesh);
}

void Draw::DestroyDrawResource(const VkDevice& device, const Render& render) {
  pipeline.DestroyPipeline(device);
  for (Mesh* mesh : meshes) {
    mesh->DestroyMesh(device, render);
    mesh->Destroy();
  }
}
