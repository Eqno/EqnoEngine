#include "../include/draw.h"

#include <vector>

#include "../include/config.h"

void Draw::CreateDrawResource(const Device& device,
                              const std::string& shaderPath,
                              const VkRenderPass& renderPass) {
  pipeline.CreatePipeline(device, shader, shaderPath, renderPass);
}

void Draw::LoadDrawResource(const Device& device, const Render& render,
                            const MeshData* data) {
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
