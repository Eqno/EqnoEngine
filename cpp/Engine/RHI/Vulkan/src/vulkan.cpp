#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/RHI/Vulkan/include/vulkan.h>
#include <Engine/System/include/Application.h>
#include <Engine/System/include/BaseInput.h>
#include <Engine/Utility/include/JsonUtils.h>

#include <mutex>
#include <thread>

void Vulkan::CreateWindow(const std::string& title) {
  int width = JSON_CONFIG(Int, "DefaultWindowWidth");
  int height = JSON_CONFIG(Int, "DefaultWindowHeight");

  width = width == 0 ? VulkanConfig::DEFAULT_WINDOW_WIDTH : width;
  height = height == 0 ? VulkanConfig::DEFAULT_WINDOW_HEIGHT : height;

  window.CreateWindow(width, height, title);
}

void Vulkan::InitConfig() {
  showRenderFrameCount = JSON_CONFIG(Bool, "ShowRenderFrameCount");
  showGameFrameCount = JSON_CONFIG(Bool, "ShowGameFrameCount");

  enableMipmap = JSON_CONFIG(Bool, "EnableMipmap");
  enableZPrePass = JSON_CONFIG(Bool, "EnableZPrePass");
  enableShadowMap = JSON_CONFIG(Bool, "EnableShadowMap");
  enableDeferred = JSON_CONFIG(Bool, "EnableDeferred");
  enableShaderDebug = JSON_CONFIG(Bool, "EnableShaderDebug");

  shadowMapWidth = JSON_CONFIG(Int, "ShadowMapWidth");
  shadowMapHeight = JSON_CONFIG(Int, "ShadowMapHeight");

  msaaSamples = JSON_CONFIG(Int, "MSAAMaxSamples");
  depthBiasClamp = JSON_CONFIG(Float, "DepthBiasClamp");
  depthBiasSlopeFactor = JSON_CONFIG(Float, "DepthBiasSlopeFactor");
  depthBiasConstantFactor = JSON_CONFIG(Float, "DepthBiasConstantFactor");
}

void Vulkan::InitGraphics() {
  InitConfig();
  instance.CreateInstance(validation);
  validation.SetupMessenger(instance.GetVkInstance());
  window.CreateSurface(instance.GetVkInstance());

  device.PickPhysicalDevice(instance.GetVkInstance(), window.GetSurface());
  device.CreateLogicalDevice(window.GetSurface(), validation);

  render.CreateRenderResources(
      device, window, JSON_CONFIG(String, "SwapChainSurfaceImageFormat"),
      JSON_CONFIG(String, "SwapChainSurfaceColorSpace"));
}

void Vulkan::TriggerOnUpdate(
    std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById) {
  auto drawIter = drawsByShader.begin();
  while (drawIter != drawsByShader.end()) {
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
        if (GetEnableShadowMap()) {
          (*meshIter)->UpdateShadowMapUniformBuffers(
              device, render, render.GetCurrentFrame(), lightsById);
        }
        meshIter++;
      }
    }
    if (meshes.empty()) {
      // Destroy the draw if meshes empty
      drawIter->second->DestroyDrawResource(device.GetLogical(), render);
      drawIter->second->Destroy();
      drawIter = drawsByShader.erase(drawIter);
    } else {
      drawIter++;
    }
  }
}

void Vulkan::GetAppPointer() {
  if (auto ownerPtr = _owner.lock()) {
    appPointer = static_pointer_cast<Application>(ownerPtr).get();
  }
}

void Vulkan::ReleaseBufferLocks() {
  for (auto& buffer : bufferManager.cameraBuffers) {
    buffer.second.second.updateLock = false;
  }
  for (auto& buffer : bufferManager.materialBuffers) {
    buffer.second.second.updateLock = false;
  }
  for (auto& buffer : bufferManager.lightChannelBuffers) {
    buffer.second.second.updateLock = false;
  }
}

void Vulkan::UpdateGameDeltaTime() {
  static auto lastTime = std::chrono::steady_clock::now();
  auto nowTime = std::chrono::steady_clock::now();

  seconds duration = nowTime - lastTime;
  GameDeltaTime = duration.count();
  lastTime = nowTime;
}

void Vulkan::UpdateRenderDeltaTime() {
  static auto lastTime = std::chrono::steady_clock::now();
  auto nowTime = std::chrono::steady_clock::now();

  seconds duration = nowTime - lastTime;
  RenderDeltaTime = duration.count();
  lastTime = nowTime;
}

void Vulkan::ShowGameFrameCount() {
  static uint32_t frameCounter = 0;
  static float accumulateTime = 0;

  frameCounter += 1;
  accumulateTime += GameDeltaTime;

  if (accumulateTime >= oneSecondTime) {
    std::cout << "Current game frame count: " << frameCounter << "\n";
    accumulateTime -= oneSecondTime;
    frameCounter = 0;
  }
}

void Vulkan::ShowRenderFrameCount() {
  static uint32_t frameCounter = 0;
  static float accumulateTime = 0;

  frameCounter += 1;
  accumulateTime += RenderDeltaTime;

  if (accumulateTime >= oneSecondTime) {
    std::cout << "Current render frame count: " << frameCounter << "\n";
    accumulateTime -= oneSecondTime;
    frameCounter = 0;
  }
}

void Vulkan::GameLoop() {
  while (GetRenderLoopEnd() == false) {
    UpdateGameDeltaTime();
    if (showGameFrameCount == true) {
      ShowGameFrameCount();
    }

    Input::RecordDownUpFlags();
    appPointer->TriggerOnUpdate();
    Input::ResetDownUpFlags();
  }
}

void Vulkan::RenderLoop() {
  GetAppPointer();
  if (appPointer == nullptr) {
    return;
  }
  SetRenderLoopEnd(false);
  std::thread(&Vulkan::GameLoop, this).detach();
  while (!glfwWindowShouldClose(window.window)) {
    if (showRenderFrameCount == true) {
      UpdateRenderDeltaTime();
      ShowRenderFrameCount();
    }
    ParseMeshData();
    TriggerOnUpdate(appPointer->GetLightsById());
    ReleaseBufferLocks();

    glfwPollEvents();
    render.DrawFrame(device, drawsByShader, appPointer->GetLightsById(),
                     window);
    device.WaitIdle();
  }
  SetRenderLoopEnd(true);
}

void Vulkan::CleanupGraphics() {
  auto drawIter = drawsByShader.begin();
  while (drawIter != drawsByShader.end()) {
    drawIter->second->DestroyDrawResource(device.GetLogical(), render);
    drawIter->second->Destroy();
    drawIter = drawsByShader.begin();
  }

  render.DestroyRenderResources(device);
  device.DestroyLogicalDevice();
  validation.DestroyMessenger(instance.GetVkInstance());

  window.DestroySurface(instance.GetVkInstance());
  instance.DestroyInstance();
  window.DestroyWindow();
}

void Vulkan::ParseMeshData() {
  if (needToUpdateMeshDatas.exchange(false)) {
    updateMeshDataMutex.lock();

    while (meshDataQueue.empty() == false) {
      if (auto mesh = meshDataQueue.front().lock()) {
        if (auto materialPtr = mesh->uniform.material.lock()) {
          std::vector<std::string>& shaders = materialPtr->GetShaders();
          if (shaders.empty()) {
            continue;
          }

          if (drawsByShader.contains(shaders[0])) {
            drawsByShader[shaders[0]]->LoadDrawResource(device, render, mesh);
          } else {
            int findFallbackIndex = -1;
            for (int i = 1; i < shaders.size(); i++) {
              if (drawsByShader.contains(shaders[i])) {
                findFallbackIndex = i;
                break;
              }
            }

            Draw* draw = Base::Create<Draw>(
                device, render, GetRoot(), mesh->textures.size(), shaders,
                JSON_CONFIG(String, "ZPrePassShaderPath"),
                JSON_CONFIG(String, "ShadowMapShaderPath"));
            int createFallbackIndex = draw->GetShaderFallbackIndex();

            if (findFallbackIndex != -1 &&
                findFallbackIndex < createFallbackIndex) {
              drawsByShader[shaders[findFallbackIndex]]->LoadDrawResource(
                  device, render, mesh);
              draw->DestroyDrawResource(device.GetLogical(), render);
              draw->Destroy();
            } else if (createFallbackIndex != -1) {
              draw->LoadDrawResource(device, render, mesh);

              const std::string& shaderPath = shaders[createFallbackIndex];
              draw->SetShaderPath(shaderPath);
              drawsByShader[shaderPath] = draw;

              int pipelineId = 0;
              while (pipelineId < MaxPipelineNum) {
                if (drawsByPipeline.contains(pipelineId) == false) {
                  draw->SetPipelineId(render, pipelineId);
                  drawsByPipeline[pipelineId] = draw;
                  break;
                }
                pipelineId++;
              }
              if (pipelineId >= MaxPipelineNum) {
                throw std::runtime_error("pipeline num exceeds maximum!");
              }
            } else {
              throw std::runtime_error("could not fallback to a valid shader!");
            }
          }
        }
      }
      meshDataQueue.pop();
    }
    updateMeshDataMutex.unlock();
  }
}

void Vulkan::ParseMeshData(std::weak_ptr<MeshData> meshData) {
  updateMeshDataMutex.lock();
  needToUpdateMeshDatas = true;
  meshDataQueue.emplace(meshData);
  updateMeshDataMutex.unlock();
}

float Vulkan::GetViewportAspect() { return render.GetViewportAspect(); }
std::weak_ptr<LightChannel> Vulkan::GetLightChannelByName(
    const std::string& name) {
  GetAppPointer();
  return appPointer->GetLightChannelByName(name);
}