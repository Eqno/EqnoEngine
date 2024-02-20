#include "../include/vulkan.h"

#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/System/include/Application.h>
#include <Engine/System/include/BaseInput.h>
#include <Engine/Utility/include/JsonUtils.h>

void Vulkan::CreateWindow(const std::string& title) {
  int width = JSON_CONFIG(Int, "DefaultWindowWidth");
  int height = JSON_CONFIG(Int, "DefaultWindowHeight");

  width = width == 0 ? VulkanConfig::DEFAULT_WINDOW_WIDTH : width;
  height = height == 0 ? VulkanConfig::DEFAULT_WINDOW_HEIGHT : height;

  window.CreateWindow(width, height, title);
}

void Vulkan::InitGraphics() {
  instance.CreateInstance(validation);
  validation.SetupMessenger(instance.GetVkInstance());
  window.CreateSurface(instance.GetVkInstance());

  device.PickPhysicalDevice(instance.GetVkInstance(), window.GetSurface());
  device.CreateLogicalDevice(window.GetSurface(), validation);

  render.CreateRenderResources(
      JSON_CONFIG(String, "SwapChainSurfaceImageFormat"),
      JSON_CONFIG(String, "SwapChainSurfaceColorSpace"),
      JSON_CONFIG(Int, "ShadowMapWidth"), JSON_CONFIG(Int, "ShadowMapHeight"),
      device, window);
}

void Vulkan::TriggerOnUpdate() {
  auto drawIter = draws.begin();
  while (drawIter != draws.end()) {
    auto& meshes = drawIter->second->GetMeshes();
    auto meshIter = meshes.begin();
    while (meshIter != meshes.end()) {
      if ((*meshIter)->GetAlive() == false) {
        // Destroy the mesh if not alive
        (*meshIter)->DestroyMesh(device.GetLogical(), render);
        (*meshIter)->Destroy();
        meshIter = meshes.erase(meshIter);
      } else {
        // Update uniform buffer if mesh alive
        (*meshIter)->UpdateUniformBuffer(render.GetCurrentFrame());
        meshIter++;
      }
    }
    if (meshes.empty()) {
      // Destroy the draw if meshes empty
      drawIter->second->DestroyDrawResource(device.GetLogical(), render);
      drawIter->second->Destroy();
      drawIter = draws.erase(drawIter);
    } else {
      drawIter++;
    }
  }
}

void Vulkan::RendererLoop() {
  while (!glfwWindowShouldClose(window.window)) {
    static auto lastTime = std::chrono::steady_clock::now();
    auto nowTime = std::chrono::steady_clock::now();

    seconds duration = nowTime - lastTime;
    DeltaTime = duration.count();
    lastTime = nowTime;

    glfwPollEvents();
    render.DrawFrame(device, draws, window);
    device.WaitIdle();

    Input::RecordDownUpFlags();
    if (auto ownerPtr = _owner.lock()) {
      std::dynamic_pointer_cast<Application>(ownerPtr)->TriggerOnUpdate();
    }
    TriggerOnUpdate();
    Input::ResetDownUpFlags();
  }
}

void Vulkan::CleanupGraphics() {
  for (const auto& val : draws | std::views::values) {
    val->DestroyDrawResource(device.GetLogical(), render);
    val->Destroy();
  }

  render.DestroyRenderResources(device.GetLogical());
  device.DestroyLogicalDevice();
  validation.DestroyMessenger(instance.GetVkInstance());

  window.DestroySurface(instance.GetVkInstance());
  instance.DestroyInstance();
  window.DestroyWindow();
}

void Vulkan::ParseMeshDatas(std::vector<std::weak_ptr<MeshData>>& meshDatas) {
  for (std::weak_ptr<MeshData> meshData : meshDatas) {
    auto mesh = meshData.lock();
    if (!mesh) {
      return;
    }
    if (auto materialPtr = mesh->uniform.material.lock()) {
      std::vector<std::string>& shaders = materialPtr->GetShaders();
      if (shaders.empty()) {
        continue;
      }

      mesh->uniform.bufferManager = &bufferManager;
      if (draws.contains(shaders[0])) {
        draws[shaders[0]]->LoadDrawResource(device, render, mesh);
      } else {
        int findFallbackIndex = -1;
        for (int i = 1; i < shaders.size(); i++) {
          if (draws.contains(shaders[i])) {
            findFallbackIndex = i;
            break;
          }
        }

        Draw* draw =
            Base::Create<Draw>(device, GetRoot(), shaders,
                               render.GetRenderPass(), mesh->textures.size());
        int createFallbackIndex = draw->GetShaderFallbackIndex();

        if (findFallbackIndex != -1 &&
            findFallbackIndex < createFallbackIndex) {
          draws[shaders[findFallbackIndex]]->LoadDrawResource(device, render,
                                                              mesh);
          draw->DestroyDrawResource(device.GetLogical(), render);
          draw->Destroy();
        } else if (createFallbackIndex != -1) {
          draw->LoadDrawResource(device, render, mesh);
          draws[shaders[createFallbackIndex]] = draw;
        } else {
          throw std::runtime_error("could not fallback to a valid shader!");
        }
      }
    }
  }
}

float Vulkan::GetViewportAspect() { return render.GetViewportAspect(); }