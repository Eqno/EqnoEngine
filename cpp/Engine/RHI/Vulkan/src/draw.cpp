#include "../include/draw.h"

#include <vector>

#include "../include/config.h"

void Draw::CreateDrawResource(const Device& device, Render& render,
                              const std::string& rootPath, const int texCount,
                              const std::vector<std::string>& shaderPaths,
                              const std::string& zPrePassShaderPath,
                              const std::string& shadowMapShaderPath) {
  shader.AddDefinitions({{"MaxLightNum", std::to_string(MaxLightNum)}});
  std::vector<std::string> shaderSearchPaths;
  for (const std::string& searchPath : ShaderSearchPaths) {
    shaderSearchPaths.push_back(rootPath + searchPath);
  }
  shader.SetFileIncluder(shaderSearchPaths);
  shader.SetOptimizationLevel(ShaderOptimizationLevel);
  pipeline.CreatePipeline(device, render, shader, texCount, rootPath,
                          shaderPaths, zPrePassShaderPath, shadowMapShaderPath);
}

void Draw::LoadDrawResource(const Device& device, Render& render,
                            std::weak_ptr<MeshData> data) {
  Mesh* mesh =
      Create<Mesh>(device, render, data, pipeline.GetColorDescriptorSetLayout(),
                   pipeline.GetZPrePassDescriptorSetLayout(),
                   pipeline.GetShadowMapDescriptorSetLayout());
  meshes.emplace_back(mesh);
}

void Draw::DestroyDrawResource(const VkDevice& device, const Render& render) {
  pipeline.DestroyPipeline(device, render);
  for (Mesh* mesh : meshes) {
    mesh->DestroyMesh(device, render);
    mesh->Destroy();
  }
}
