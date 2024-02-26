#include "../include/render.h"

#include <Engine/Light/include/BaseLight.h>

#include <ranges>
#include <stdexcept>

#include "../include/depth.h"
#include "../include/device.h"
#include "../include/draw.h"
#include "../include/mesh.h"
#include "../include/swapchain.h"
#include "../include/vertex.h"

void Render::CreateColorRenderPass(const Device& device) {
  const VkAttachmentDescription colorAttachment{
      .format = swapChain.GetImageFormat(),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  constexpr VkAttachmentReference colorAttachmentRef{
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  const VkAttachmentDescription depthAttachment{
      .format = swapChain.GetZPrePassDepthFormat(),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
  constexpr VkAttachmentReference depthAttachmentRef{
      .attachment = 1,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
  VkSubpassDescription subPass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
      .pDepthStencilAttachment = &depthAttachmentRef,
  };
  constexpr VkSubpassDependency depBegToDepthBuffer{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
  };
  constexpr VkSubpassDependency depEndFromDepthBuffer{
      .srcSubpass = 0,
      .dstSubpass = VK_SUBPASS_EXTERNAL,
      .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
  };
  constexpr VkSubpassDependency depBegToColorBuffer{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };
  constexpr VkSubpassDependency depEndFromColorBuffer{
      .srcSubpass = 0,
      .dstSubpass = VK_SUBPASS_EXTERNAL,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
  };
  std::array attachments = {colorAttachment, depthAttachment};
  std::array dependencies = {depBegToDepthBuffer, depEndFromDepthBuffer,
                             depBegToColorBuffer, depEndFromColorBuffer};
  const VkRenderPassCreateInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subPass,
      .dependencyCount = static_cast<uint32_t>(dependencies.size()),
      .pDependencies = dependencies.data(),
  };
  if (vkCreateRenderPass(device.GetLogical(), &renderPassInfo, nullptr,
                         &colorRenderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create color render pass!");
  }
}

void Render::CreateZPrePassRenderPass(const Device& device) {
  const VkAttachmentDescription depthAttachment{
      .format = swapChain.GetZPrePassDepthFormat(),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
  constexpr VkAttachmentReference depthAttachmentRef{
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subPass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 0,
      .pColorAttachments = nullptr,
      .pDepthStencilAttachment = &depthAttachmentRef,
  };
  constexpr VkSubpassDependency depBegToDepthBuffer{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
  };
  constexpr VkSubpassDependency depEndFromDepthBuffer{
      .srcSubpass = 0,
      .dstSubpass = VK_SUBPASS_EXTERNAL,
      .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
  };
  std::array dependencies{depBegToDepthBuffer, depEndFromDepthBuffer};
  const VkRenderPassCreateInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &depthAttachment,
      .subpassCount = 1,
      .pSubpasses = &subPass,
      .dependencyCount = static_cast<uint32_t>(dependencies.size()),
      .pDependencies = dependencies.data(),
  };
  if (vkCreateRenderPass(device.GetLogical(), &renderPassInfo, nullptr,
                         &zPrePassRenderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create z pre pass render pass!");
  }
}

void Render::CreateShadowMapRenderPass(const Device& device) {
  const VkAttachmentDescription depthAttachment{
      .format = swapChain.GetShadowMapDepthFormat(),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
  constexpr VkAttachmentReference depthAttachmentRef{
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subPass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 0,
      .pColorAttachments = nullptr,
      .pDepthStencilAttachment = &depthAttachmentRef,
  };
  constexpr VkSubpassDependency depBegToDepthBuffer{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
      .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  };
  constexpr VkSubpassDependency depEndFromDepthBuffer{
      .srcSubpass = 0,
      .dstSubpass = VK_SUBPASS_EXTERNAL,
      .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
      .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
  };
  std::array dependencies{depBegToDepthBuffer, depEndFromDepthBuffer};
  const VkRenderPassCreateInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &depthAttachment,
      .subpassCount = 1,
      .pSubpasses = &subPass,
      .dependencyCount = static_cast<uint32_t>(dependencies.size()),
      .pDependencies = dependencies.data(),
  };
  if (vkCreateRenderPass(device.GetLogical(), &renderPassInfo, nullptr,
                         &shadowMapRenderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shadow map render pass!");
  }
}

void Render::CreateRenderPasses(const Device& device) {
  CreateColorRenderPass(device);
  CreateZPrePassRenderPass(device);
  CreateShadowMapRenderPass(device);
}

void Render::CreateCommandPool(const Device& device,
                               const VkSurfaceKHR& surface) {
  const auto [graphicsFamily, presentFamily] =
      device.FindQueueFamilies(surface);

  const VkCommandPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex =
          graphicsFamily.has_value() ? graphicsFamily.value() : 0,
  };

  if (vkCreateCommandPool(device.GetLogical(), &poolInfo, nullptr,
                          &commandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics command pool!");
  }
}

void Render::CreateCommandBuffers(
    const VkDevice& device, std::vector<VkCommandBuffer>& commandBuffers) {
  commandBuffers.resize(maxFramesInFlight);

  const VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
  };

  if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void Render::CreateCommandBuffersSet(const VkDevice& device) {
  CreateCommandBuffers(device, colorCommandBuffers);
  CreateCommandBuffers(device, zPrePassCommandBuffers);

  shadowMapCommandBuffers.resize(GetShadowMapDepthNum());
  for (int i = 0; i < GetShadowMapDepthNum(); i++) {
    CreateCommandBuffers(device, shadowMapCommandBuffers[i]);
  }
}

void Render::CopyCommandBuffer(const Device& device, const VkBuffer& srcBuffer,
                               const VkBuffer& dstBuffer,
                               const VkDeviceSize& size) const {
  VkCommandBuffer commandBuffer;
  BeginSingleTimeCommands(device.GetLogical(), &commandBuffer);
  const VkBufferCopy copyRegion{
      .size = size,
  };
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
  EndSingleTimeCommands(device, &commandBuffer);
}

void Render::BeginSingleTimeCommands(const VkDevice& device,
                                     VkCommandBuffer* commandBuffer) const {
  const VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  vkAllocateCommandBuffers(device, &allocInfo, commandBuffer);

  constexpr VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(*commandBuffer, &beginInfo);
}

void Render::EndSingleTimeCommands(const Device& device,
                                   VkCommandBuffer* commandBuffer) const {
  vkEndCommandBuffer(*commandBuffer);
  const VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = commandBuffer,
  };

  const auto& graphicsQueue = device.GetGraphicsQueue();
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);
  vkFreeCommandBuffers(device.GetLogical(), commandPool, 1, commandBuffer);
}

void Render::RecordZPrePassCommandBuffer(
    const Device& device, std::unordered_map<std::string, Draw*>& draws) {
  const VkCommandBuffer& commandBuffer = zPrePassCommandBuffers[currentFrame];

  constexpr VkCommandBufferBeginInfo commandBufferBeginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };
  if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  constexpr std::array clearValues{
      VkClearValue{.depthStencil = {1.0f, 0}},
  };
  VkRenderPassBeginInfo renderPassBeginInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = zPrePassRenderPass,
      .framebuffer = swapChain.GetZPrePassFrameBuffer(),
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = swapChain.GetExtent();

  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  const VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapChain.GetExtent().width),
      .height = static_cast<float>(swapChain.GetExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  const VkRect2D scissor{
      .offset = {0, 0},
      .extent = swapChain.GetExtent(),
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  for (Draw* draw : draws | std::views::values) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      draw->GetZPrePassGraphicsPipeline());

    for (const auto& mesh : draw->GetMeshes()) {
      const VkBuffer vertexBuffers[] = {mesh->GetVertexBuffer()};
      constexpr VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

      vkCmdBindIndexBuffer(commandBuffer, mesh->GetIndexBuffer(), 0,
                           VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(
          commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          draw->GetZPrePassPipelineLayout(), 0, 1,
          &mesh->GetZPrePassDescriptorSetByIndex(currentFrame), 0, nullptr);

      vkCmdDrawIndexed(commandBuffer,
                       static_cast<uint32_t>(mesh->GetIndices().size()), 1, 0,
                       0, 0);
    }
  }
  vkCmdEndRenderPass(commandBuffer);

  // Convert shader source to shadow map depth
  for (uint32_t i = 0; i < GetShadowMapDepthNum(); i++) {
    swapChain.GetShadowMapDepthByIndex(i).TransitionDepthImageLayout(
        device, *this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBuffer);
  }
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void Render::RecordShadowMapCommandBuffer(
    const Device& device, std::unordered_map<std::string, Draw*>& draws,
    BaseLight* light) {
  const VkCommandBuffer& commandBuffer =
      shadowMapCommandBuffers[light->GetId()][currentFrame];

  constexpr VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  constexpr std::array clearValues{
      VkClearValue{.depthStencil = {1.0f, 0}},
  };
  VkRenderPassBeginInfo renderPassBeginInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = shadowMapRenderPass,
      .framebuffer = swapChain.GetShadowMapFrameBufferByIndex(light->GetId()),
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = {swapChain.GetShadowMapWidth(),
                                           swapChain.GetShadowMapHeight()};

  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  const VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapChain.GetShadowMapWidth()),
      .height = static_cast<float>(swapChain.GetShadowMapHeight()),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  const VkRect2D scissor{
      .offset = {0, 0},
      .extent = {swapChain.GetShadowMapWidth(), swapChain.GetShadowMapHeight()},
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp,
                    depthBiasSlopeFactor);

  for (Draw* draw : draws | std::views::values) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      draw->GetShadowMapGraphicsPipeline());

    for (const auto& mesh : draw->GetMeshes()) {
      mesh->UpdateShadowMapUniformBuffer(currentFrame, light);

      const VkBuffer vertexBuffers[] = {mesh->GetVertexBuffer()};
      constexpr VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

      vkCmdBindIndexBuffer(commandBuffer, mesh->GetIndexBuffer(), 0,
                           VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(
          commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          draw->GetShadowMapPipelineLayout(), 0, 1,
          &mesh->GetShadowMapDescriptorSetByIndex(currentFrame), 0, nullptr);

      vkCmdDrawIndexed(commandBuffer,
                       static_cast<uint32_t>(mesh->GetIndices().size()), 1, 0,
                       0, 0);
    }
  }
  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void Render::RecordColorCommandBuffer(
    const Device& device, std::unordered_map<std::string, Draw*>& draws,
    const uint32_t imageIndex) {
  const VkCommandBuffer& commandBuffer = colorCommandBuffers[currentFrame];

  constexpr VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  // Convert shadow map depth to shader source
  for (uint32_t i = 0; i < GetShadowMapDepthNum(); i++) {
    swapChain.GetShadowMapDepthByIndex(i).TransitionDepthImageLayout(
        device, *this, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
  }

  constexpr std::array clearValues{
      VkClearValue{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
      VkClearValue{.depthStencil = {1.0f, 0}},
  };
  VkRenderPassBeginInfo renderPassBeginInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = colorRenderPass,
      .framebuffer = swapChain.GetColorFrameBufferByIndex(imageIndex),
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = swapChain.GetExtent();

  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  const VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapChain.GetExtent().width),
      .height = static_cast<float>(swapChain.GetExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  const VkRect2D scissor{
      .offset = {0, 0},
      .extent = swapChain.GetExtent(),
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  for (Draw* draw : draws | std::views::values) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      draw->GetColorGraphicsPipeline());

    for (const auto& mesh : draw->GetMeshes()) {
      const VkBuffer vertexBuffers[] = {mesh->GetVertexBuffer()};
      constexpr VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

      vkCmdBindIndexBuffer(commandBuffer, mesh->GetIndexBuffer(), 0,
                           VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              draw->GetColorPipelineLayout(), 0, 1,
                              &mesh->GetColorDescriptorSetByIndex(currentFrame),
                              0, nullptr);

      vkCmdDrawIndexed(commandBuffer,
                       static_cast<uint32_t>(mesh->GetIndices().size()), 1, 0,
                       0, 0);
    }
  }
  vkCmdEndRenderPass(commandBuffer);
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void Render::SubmitCommandBuffer(const Device& device,
                                 const VkSemaphore& waitSemaphore,
                                 const VkCommandBuffer& commandBuffer,
                                 const VkSemaphore& signalSemaphore,
                                 const VkFence waitFence) {
  constexpr VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  const VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &waitSemaphore,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &signalSemaphore,
  };
  if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, waitFence) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit command buffer!");
  }
}

void Render::WaitFences(
    const Device& device,
    std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById) {
  vkWaitForFences(device.GetLogical(), 1, &colorInFlightFences[currentFrame],
                  VK_TRUE, UINT64_MAX);
  vkWaitForFences(device.GetLogical(), 1, &zPrePassInFlightFences[currentFrame],
                  VK_TRUE, UINT64_MAX);
  auto iter = lightsById.begin();
  while (iter != lightsById.end()) {
    if (auto lightPtr = iter->second.lock()) {
      vkWaitForFences(device.GetLogical(), 1,
                      &shadowMapInFlightFences[lightPtr->GetId()][currentFrame],
                      VK_TRUE, UINT64_MAX);
      iter++;
    } else {
      iter = lightsById.erase(iter);
    }
  }
}

void Render::ResetFences(
    const Device& device,
    std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById) {
  vkResetFences(device.GetLogical(), 1, &colorInFlightFences[currentFrame]);
  vkResetFences(device.GetLogical(), 1, &zPrePassInFlightFences[currentFrame]);
  auto iter = lightsById.begin();
  while (iter != lightsById.end()) {
    if (auto lightPtr = iter->second.lock()) {
      vkResetFences(device.GetLogical(), 1,
                    &shadowMapInFlightFences[lightPtr->GetId()][currentFrame]);
      iter++;
    } else {
      iter = lightsById.erase(iter);
    }
  }
}

void Render::DrawFrame(
    const Device& device, std::unordered_map<std::string, Draw*>& draws,
    std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById,
    Window& window) {
  WaitFences(device, lightsById);
  uint32_t imageIndex;
  auto result = vkAcquireNextImageKHR(
      device.GetLogical(), swapChain.Get(), UINT64_MAX,
      imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    swapChain.RecreateSwapChain(device, window, draws, colorRenderPass,
                                zPrePassRenderPass, shadowMapRenderPass);
    return;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  ResetFences(device, lightsById);

  vkResetCommandBuffer(zPrePassCommandBuffers[currentFrame],
                       /*VkCommandBufferResetFlagBits*/
                       0);
  RecordZPrePassCommandBuffer(device, draws);
  SubmitCommandBuffer(device, imageAvailableSemaphores[currentFrame],
                      zPrePassCommandBuffers[currentFrame],
                      zPrePassFinishedSemaphores[currentFrame],
                      zPrePassInFlightFences[currentFrame]);

  VkSemaphore lastSemaphore = zPrePassFinishedSemaphores[currentFrame];
  auto iter = lightsById.begin();
  while (iter != lightsById.end()) {
    if (auto lightPtr = iter->second.lock()) {
      vkResetCommandBuffer(
          shadowMapCommandBuffers[lightPtr->GetId()][currentFrame],
          /*VkCommandBufferResetFlagBits*/
          0);
      RecordShadowMapCommandBuffer(device, draws, lightPtr.get());
      VkSemaphore nextSemaphore =
          shadowMapFinishedSemaphores[lightPtr->GetId()][currentFrame];
      SubmitCommandBuffer(
          device, lastSemaphore,
          shadowMapCommandBuffers[lightPtr->GetId()][currentFrame],
          nextSemaphore,
          shadowMapInFlightFences[lightPtr->GetId()][currentFrame]);
      lastSemaphore = nextSemaphore;
      iter++;
    } else {
      iter = lightsById.erase(iter);
    }
  }

  vkResetCommandBuffer(colorCommandBuffers[currentFrame],
                       /*VkCommandBufferResetFlagBits*/
                       0);
  RecordColorCommandBuffer(device, draws, imageIndex);
  SubmitCommandBuffer(device, lastSemaphore, colorCommandBuffers[currentFrame],
                      renderFinishedSemaphores[currentFrame],
                      colorInFlightFences[currentFrame]);

  const VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
      .swapchainCount = 1,
      .pSwapchains = &swapChain.Get(),
      .pImageIndices = &imageIndex,
  };
  result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      window.GetFrameBufferResized()) {
    window.SetFrameBufferResized(false);
    swapChain.RecreateSwapChain(device, window, draws, colorRenderPass,
                                zPrePassRenderPass, shadowMapRenderPass);
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
  currentFrame = (currentFrame + 1) % maxFramesInFlight;
}

void Render::CreateSyncObjects(const VkDevice& device) {
  shadowMapFinishedSemaphores.resize(GetShadowMapDepthNum());
  shadowMapInFlightFences.resize(GetShadowMapDepthNum());

  imageAvailableSemaphores.resize(maxFramesInFlight);
  zPrePassFinishedSemaphores.resize(maxFramesInFlight);
  renderFinishedSemaphores.resize(maxFramesInFlight);

  colorInFlightFences.resize(maxFramesInFlight);
  zPrePassInFlightFences.resize(maxFramesInFlight);

  for (int i = 0; i < GetShadowMapDepthNum(); i++) {
    shadowMapFinishedSemaphores[i].resize(maxFramesInFlight);
    shadowMapInFlightFences[i].resize(maxFramesInFlight);
  }

  constexpr VkSemaphoreCreateInfo semaphoreInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  constexpr VkFenceCreateInfo fenceInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  for (int i = 0; i < maxFramesInFlight; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &zPrePassFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &colorInFlightFences[i]) !=
            VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr,
                      &zPrePassInFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render semaphores or fences!");
    }
    for (int j = 0; j < GetShadowMapDepthNum(); j++) {
      if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                            &shadowMapFinishedSemaphores[j][i]) != VK_SUCCESS ||
          vkCreateFence(device, &fenceInfo, nullptr,
                        &shadowMapInFlightFences[j][i]) != VK_SUCCESS) {
        throw std::runtime_error(
            "failed to create render semaphores or fences!");
      }
    }
  }
}

void Render::DestroyRenderPasses(const VkDevice& device) const {
  vkDestroyRenderPass(device, colorRenderPass, nullptr);
  vkDestroyRenderPass(device, zPrePassRenderPass, nullptr);
  vkDestroyRenderPass(device, shadowMapRenderPass, nullptr);
}

void Render::DestroyCommandPool(const VkDevice& device) const {
  vkDestroyCommandPool(device, commandPool, nullptr);
}

void Render::DestroySyncObjects(const VkDevice& device) {
  for (int i = 0; i < maxFramesInFlight; i++) {
    vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device, zPrePassFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);

    for (int j = 0; j < GetShadowMapDepthNum(); j++) {
      vkDestroySemaphore(device, shadowMapFinishedSemaphores[j][i], nullptr);
      vkDestroyFence(device, shadowMapInFlightFences[j][i], nullptr);
    }

    vkDestroyFence(device, colorInFlightFences[i], nullptr);
    vkDestroyFence(device, zPrePassInFlightFences[i], nullptr);
  }
}