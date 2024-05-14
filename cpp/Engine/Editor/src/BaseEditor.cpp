#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Editor/include/BaseEditor.h>
#include <Engine/Light/include/SpotLight.h>
#include <Engine/Light/include/SunLight.h>
#include <Engine/Model/include/BaseModel.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/System/include/Application.h>
#include <Engine/System/include/BaseInput.h>
#include <stdio.h>   // printf, fprintf
#include <stdlib.h>  // abort

#include <algorithm>
#include <cmath>

#define DoubleClickTimeInterval 0.3f

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// #define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// Data
static VkAllocationCallbacks* g_Allocator = nullptr;
static VkInstance g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice g_Device = VK_NULL_HANDLE;
static uint32_t g_QueueFamily = (uint32_t)-1;
static VkQueue g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int g_MinImageCount = 2;
static bool g_SwapChainRebuild = false;

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void check_vk_result(VkResult err) {
  if (err == 0) return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0) abort();
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
             uint64_t object, size_t location, int32_t messageCode,
             const char* pLayerPrefix, const char* pMessage, void* pUserData) {
  (void)flags;
  (void)object;
  (void)location;
  (void)messageCode;
  (void)pUserData;
  (void)pLayerPrefix;  // Unused arguments
  fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n",
          objectType, pMessage);
  return VK_FALSE;
}
#endif  // APP_USE_VULKAN_DEBUG_REPORT

static bool IsExtensionAvailable(
    const ImVector<VkExtensionProperties>& properties, const char* extension) {
  for (const VkExtensionProperties& p : properties)
    if (strcmp(p.extensionName, extension) == 0) return true;
  return false;
}

static VkPhysicalDevice SetupVulkan_SelectPhysicalDevice() {
  uint32_t gpu_count;
  VkResult err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, nullptr);
  check_vk_result(err);
  IM_ASSERT(gpu_count > 0);

  ImVector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus.Data);
  check_vk_result(err);

  // If a number >1 of GPUs got reported, find discrete GPU if present, or use
  // first one available. This covers most common cases
  // (multi-gpu/integrated+dedicated graphics). Handling more complicated setups
  // (multiple dedicated GPUs) is out of scope of this sample.
  for (VkPhysicalDevice& device : gpus) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      return device;
  }

  // Use first GPU (Integrated) is a Discrete one is not available.
  if (gpu_count > 0) return gpus[0];
  return VK_NULL_HANDLE;
}

static void SetupVulkan(ImVector<const char*> instance_extensions) {
  VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
  volkInitialize();
#endif

  // Create Vulkan Instance
  {
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Enumerate available extensions
    uint32_t properties_count;
    ImVector<VkExtensionProperties> properties;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count,
                                                 properties.Data);
    check_vk_result(err);

    // Enable required extensions
    if (IsExtensionAvailable(
            properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
      instance_extensions.push_back(
          VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (IsExtensionAvailable(properties,
                             VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
      instance_extensions.push_back(
          VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = layers;
    instance_extensions.push_back("VK_EXT_debug_report");
#endif

    // Create Vulkan Instance
    create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
    create_info.ppEnabledExtensionNames = instance_extensions.Data;
    err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
    check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkLoadInstance(g_Instance);
#endif

    // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    auto f_vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            g_Instance, "vkCreateDebugReportCallbackEXT");
    IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
    debug_report_ci.sType =
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                            VK_DEBUG_REPORT_WARNING_BIT_EXT |
                            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debug_report_ci.pfnCallback = debug_report;
    debug_report_ci.pUserData = nullptr;
    err = f_vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci,
                                           g_Allocator, &g_DebugReport);
    check_vk_result(err);
#endif
  }

  // Select Physical Device (GPU)
  g_PhysicalDevice = SetupVulkan_SelectPhysicalDevice();

  // Select graphics queue family
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, nullptr);
    VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(
        sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
    for (uint32_t i = 0; i < count; i++)
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        g_QueueFamily = i;
        break;
      }
    free(queues);
    IM_ASSERT(g_QueueFamily != (uint32_t)-1);
  }

  // Create Logical Device (with 1 queue)
  {
    ImVector<const char*> device_extensions;
    device_extensions.push_back("VK_KHR_swapchain");

    // Enumerate physical device extension
    uint32_t properties_count;
    ImVector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr,
                                         &properties_count, nullptr);
    properties.resize(properties_count);
    vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr,
                                         &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(properties,
                             VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
      device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = g_QueueFamily;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount =
        sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
    create_info.ppEnabledExtensionNames = device_extensions.Data;
    err =
        vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
    check_vk_result(err);
    vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
  }

  // Create Descriptor Pool
  // The example only requires a single combined image sampler descriptor for
  // the font image and only uses one descriptor set (for that) If you wish to
  // load e.g. additional textures you may need to alter pools sizes.
  {
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator,
                                 &g_DescriptorPool);
    check_vk_result(err);
  }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used
// by the demo. Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd,
                              VkSurfaceKHR surface, int width, int height) {
  wd->Surface = surface;

  // Check for WSI support
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily,
                                       wd->Surface, &res);
  if (res != VK_TRUE) {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM,
                                                VK_FORMAT_R8G8B8A8_UNORM};
  const VkColorSpaceKHR requestSurfaceColorSpace =
      VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
      g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat,
      (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
      requestSurfaceColorSpace);

  // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR,
                                      VK_PRESENT_MODE_IMMEDIATE_KHR,
                                      VK_PRESENT_MODE_FIFO_KHR};
#else
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
  wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
      g_PhysicalDevice, wd->Surface, &present_modes[0],
      IM_ARRAYSIZE(present_modes));
  // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

  // Create SwapChain, RenderPass, Framebuffer, etc.
  IM_ASSERT(g_MinImageCount >= 2);
  ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device,
                                         wd, g_QueueFamily, g_Allocator, width,
                                         height, g_MinImageCount);
}

static void CleanupVulkan() {
  vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
  // Remove the debug report callback
  auto f_vkDestroyDebugReportCallbackEXT =
      (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          g_Instance, "vkDestroyDebugReportCallbackEXT");
  f_vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif  // APP_USE_VULKAN_DEBUG_REPORT

  vkDestroyDevice(g_Device, g_Allocator);
  vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow() {
  ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData,
                                  g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
  VkResult err;

  VkSemaphore image_acquired_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX,
                              image_acquired_semaphore, VK_NULL_HANDLE,
                              &wd->FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_SwapChainRebuild = true;
    return;
  }
  check_vk_result(err);

  ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
  {
    err = vkWaitForFences(
        g_Device, 1, &fd->Fence, VK_TRUE,
        UINT64_MAX);  // wait indefinitely instead of periodically checking
    check_vk_result(err);

    err = vkResetFences(g_Device, 1, &fd->Fence);
    check_vk_result(err);
  }
  {
    err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    check_vk_result(err);
  }
  {
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = wd->RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.extent.width = wd->Width;
    info.renderArea.extent.height = wd->Height;
    info.clearValueCount = 1;
    info.pClearValues = &wd->ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  // Record dear imgui primitives into command buffer
  ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

  // Submit command buffer
  vkCmdEndRenderPass(fd->CommandBuffer);
  {
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &image_acquired_semaphore;
    info.pWaitDstStageMask = &wait_stage;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &fd->CommandBuffer;
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &render_complete_semaphore;

    err = vkEndCommandBuffer(fd->CommandBuffer);
    check_vk_result(err);
    err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
    check_vk_result(err);
  }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd) {
  if (g_SwapChainRebuild) return;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &render_complete_semaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &wd->Swapchain;
  info.pImageIndices = &wd->FrameIndex;
  VkResult err = vkQueuePresentKHR(g_Queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_SwapChainRebuild = true;
    return;
  }
  check_vk_result(err);
  wd->SemaphoreIndex =
      (wd->SemaphoreIndex + 1) %
      wd->SemaphoreCount;  // Now we can use the next set of semaphores
}

void BaseEditor::GetAppPointer() {
  if (appPointer == nullptr) {
    auto ownerPtr = _owner.lock();
    appPointer = static_pointer_cast<Application>(ownerPtr).get();
  }
}

void BaseEditor::InitConfig() {
  LaunchSceneInEditor = JSON_CONFIG(Bool, "LaunchSceneInEditor");
  editorWindowWidth = JSON_CONFIG(Int, "DefaultWindowWidth");
  editorWindowHeight = JSON_CONFIG(Int, "DefaultWindowHeight");

  if (appPointer->GetLaunchSceneInEditor()) {
    std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
        appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
    std::string appConfigPath = JsonUtils::ReadStringFromFile(
        appPointer->GetRoot() + appPointer->GetFile(), "ApplicationConfig");
    windowTitle = JsonUtils::ReadStringFromFile(
        appPointer->GetRoot() + appConfigPath, "ApplicationName");
    graphicsWindowWidth = JsonUtils::ReadFloatFromFile(
        appPointer->GetRoot() + graphicsConfigPath, "DefaultWindowWidth");
    graphicsWindowHeight = JsonUtils::ReadFloatFromFile(
        appPointer->GetRoot() + graphicsConfigPath, "DefaultWindowHeight");
  } else {
    windowTitle = JSON_CONFIG(String, "EditorName");
  }

  msaaSamples = appPointer->GetMSAASamples();
  enableMipmap = appPointer->GetEnableMipmap();
  enableZPrePass = appPointer->GetEnableZPrePass();
  enableShadowMap = appPointer->GetEnableShadowMap();
  enableDeferredRendering = appPointer->GetEnableDeferred();
}

// Main code
void BaseEditor::LoadImgui() {
  GetAppPointer();
  InitConfig();

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return;

  // Create window with Vulkan context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(editorWindowWidth, editorWindowHeight,
                            windowTitle.c_str(), nullptr, nullptr);

  if (appPointer->GetLaunchSceneInEditor()) {
    parentWindow =
        glfwCreateWindow(editorWindowWidth + graphicsWindowWidth,
                         max(editorWindowHeight, graphicsWindowHeight),
                         windowTitle.c_str(), nullptr, nullptr);
    glfwSetWindowAttrib(parentWindow, GLFW_RESIZABLE, GLFW_FALSE);

    HWND editorWindowHWND = glfwGetWin32Window(window);
    HWND parentWindowHWND = glfwGetWin32Window(parentWindow);

    SetWindowLong(editorWindowHWND, GWL_STYLE, WS_VISIBLE);
    MoveWindow(editorWindowHWND, graphicsWindowWidth, 0, editorWindowWidth,
               editorWindowHeight, TRUE);
    SetParent(editorWindowHWND, parentWindowHWND);
  }

  if (!glfwVulkanSupported()) {
    printf("GLFW: Vulkan Not Supported\n");
    return;
  }

  ImVector<const char*> extensions;
  uint32_t extensions_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&extensions_count);
  for (uint32_t i = 0; i < extensions_count; i++)
    extensions.push_back(glfw_extensions[i]);
  SetupVulkan(extensions);

  // Create Window Surface
  VkSurfaceKHR surface;
  VkResult err =
      glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
  check_vk_result(err);

  // Create Framebuffers
  int w, h;
  glfwGetFramebufferSize(window, &w, &h);
  wd = &g_MainWindowData;
  SetupVulkanWindow(wd, surface, w, h);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  ioPtr = &io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = g_Instance;
  init_info.PhysicalDevice = g_PhysicalDevice;
  init_info.Device = g_Device;
  init_info.QueueFamily = g_QueueFamily;
  init_info.Queue = g_Queue;
  init_info.PipelineCache = g_PipelineCache;
  init_info.DescriptorPool = g_DescriptorPool;
  init_info.RenderPass = wd->RenderPass;
  init_info.Subpass = 0;
  init_info.MinImageCount = g_MinImageCount;
  init_info.ImageCount = wd->ImageCount;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = g_Allocator;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info);
}

void BaseEditor::UpdateImgui() {
  // Resize swap chain?
  if (g_SwapChainRebuild) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width > 0 && height > 0) {
      ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
      ImGui_ImplVulkanH_CreateOrResizeWindow(
          g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData,
          g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
      g_MainWindowData.FrameIndex = 0;
      g_SwapChainRebuild = false;
    }
  }

  // Start the Dear ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  EditorDrawMenuBar();
  EditorDrawRestartTip();
  EditorDrawFrameCount();
  EditorDrawLaunchCommand();
  EditorDrawFileExplorer();
  EditorDrawSceneHierarchy();
  EditorDrawObjectInspector();

  // Rendering
  ImGui::Render();
  ImDrawData* draw_data = ImGui::GetDrawData();
  const bool is_minimized =
      (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
  if (!is_minimized) {
    wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
    wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
    wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
    wd->ClearValue.color.float32[3] = clear_color.w;
    FrameRender(wd, draw_data);
    FramePresent(wd);
  }
}

void BaseEditor::LoopImgui() {
  if (appPointer->GetLaunchSceneInEditor()) {
    while (!glfwWindowShouldClose(parentWindow)) {
      glfwPollEvents();
      UpdateImgui();
    }
  } else {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      UpdateImgui();
    }
  }
}

void BaseEditor::DestroyImgui() {
  // Cleanup
  VkResult err = vkDeviceWaitIdle(g_Device);
  check_vk_result(err);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  CleanupVulkanWindow();
  CleanupVulkan();

  glfwDestroyWindow(window);
  if (appPointer->GetLaunchSceneInEditor()) {
    glfwDestroyWindow(parentWindow);
  }
  glfwTerminate();
}

void BaseEditor::EditorDrawMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    float menuWidth = ImGui::CalcTextSize("Graphics Settings").x +
                      ImGui::CalcTextSize("Status Display").x +
                      ImGui::CalcTextSize("Details Content").x +
                      ImGui::GetStyle().ItemSpacing.x * 8;
    float offset = (ImGui::GetWindowWidth() - menuWidth) * 0.5f;
    ImGui::SetCursorPosX(offset);
    if (ImGui::BeginMenu("Graphics Settings")) {
      ImGui::PushItemWidth(ImGui::CalcTextSize("0").x * 3);
      if (ImGui::InputInt("MSAA Samples", &msaaSamples, 0, 0,
                          ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (msaaSamples < 1) msaaSamples = 1;
        if (msaaSamples > 64) msaaSamples = 64;
        std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
            appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
        JsonUtils::ModifyIntOfFile(appPointer->GetRoot() + graphicsConfigPath,
                                   "MSAAMaxSamples", msaaSamples);
        appPointer->SetGraphicsSettingsModified(true);
      }
      ImGui::PopItemWidth();
      if (ImGui::Checkbox("Enable Mipmap", &enableMipmap)) {
        std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
            appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
        JsonUtils::ModifyBoolOfFile(appPointer->GetRoot() + graphicsConfigPath,
                                    "EnableMipmap", enableMipmap);
        appPointer->SetGraphicsSettingsModified(true);
      }
      if (ImGui::Checkbox("Enable Z-PrePass", &enableZPrePass)) {
        std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
            appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
        JsonUtils::ModifyBoolOfFile(appPointer->GetRoot() + graphicsConfigPath,
                                    "EnableZPrePass", enableZPrePass);
        appPointer->SetGraphicsSettingsModified(true);
      }
      if (ImGui::Checkbox("Enable ShadowMap", &enableShadowMap)) {
        std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
            appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
        JsonUtils::ModifyBoolOfFile(appPointer->GetRoot() + graphicsConfigPath,
                                    "EnableShadowMap", enableShadowMap);
        appPointer->SetGraphicsSettingsModified(true);
      }
      if (ImGui::Checkbox("Enable Deferred", &enableDeferredRendering)) {
        std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
            appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
        JsonUtils::ModifyBoolOfFile(appPointer->GetRoot() + graphicsConfigPath,
                                    "EnableDeferred", enableDeferredRendering);
        appPointer->SetGraphicsSettingsModified(true);
      }
      ImGui::EndMenu();
    }
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    if (ImGui::BeginMenu("Status Display")) {
      if (appPointer->GetSceneState() == SceneState::Running) {
        if (ImGui::Checkbox("Show Render FPS",
                            &appPointer->GetShowRenderFrame())) {
          std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
              appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
          JsonUtils::ModifyBoolOfFile(
              appPointer->GetRoot() + graphicsConfigPath,
              "ShowRenderFrameCount", appPointer->GetShowRenderFrame());
        }
        if (ImGui::Checkbox("Show Logic FPS",
                            &appPointer->GetShowGameFrame())) {
          std::string graphicsConfigPath = JsonUtils::ReadStringFromFile(
              appPointer->GetRoot() + appPointer->GetFile(), "GraphicsConfig");
          JsonUtils::ModifyBoolOfFile(
              appPointer->GetRoot() + graphicsConfigPath, "ShowGameFrameCount",
              appPointer->GetShowGameFrame());
        }
      } else {
        bool showRenderFrame = false, showGameFrame = false;
        ImGui::BeginDisabled(true);
        ImGui::Checkbox("Show Render FPS", &showRenderFrame);
        ImGui::Checkbox("Show Logic FPS", &showGameFrame);
        ImGui::EndDisabled();
      }
      ImGui::EndMenu();
    }
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    if (ImGui::BeginMenu("Details Content")) {
      ImGui::Checkbox("Show File Explorer", &showFileExplorer);
      ImGui::Checkbox("Show Scene Hierarchy", &showSceneHierarchy);
      ImGui::Checkbox("Show Object Inspector", &showObjectInspector);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

bool BaseEditor::GetShowRestartTip() {
  return (appPointer->GetSceneState() == SceneState::Launching ||
          appPointer->GetSceneState() == SceneState::Running ||
          appPointer->GetSceneState() == SceneState::Terminating) &&
         appPointer->GetGraphicsSettingsModified();
}

void BaseEditor::EditorDrawRestartTip() {
  if (GetShowRestartTip()) {
    float menuBarHeight = ImGui::GetFrameHeight();

    float windowHeight = 20.0f;
    ImVec2 windowPos = ImVec2(0, menuBarHeight);
    ImVec2 windowSize = ImVec2(ImGui::GetIO().DisplaySize.x, windowHeight);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("Settings Notification", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar)) {
      const char* message = "Please restart to apply changed settings";
      ImVec2 textSize = ImGui::CalcTextSize(message);
      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textSize.x) * 0.5f);
      ImGui::Text("%s", message);
      ImGui::End();
    }
  }
}

void BaseEditor::EditorDrawFrameCount() {
  if (appPointer->GetSceneState() == SceneState::Running &&
      (appPointer->GetShowRenderFrame() || appPointer->GetShowGameFrame())) {
    float fpsWindowHeight = 20.0f;
    float fpsWindowPosY = ImGui::GetFrameHeight();
    if (GetShowRestartTip()) {
      fpsWindowPosY += 32.0f;
    }
    ImGui::SetNextWindowPos(ImVec2(0, fpsWindowPosY));
    ImGui::SetNextWindowSize(
        ImVec2(ImGui::GetIO().DisplaySize.x, fpsWindowHeight));
    if (ImGui::Begin("FPS Display", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      std::string fpsText;
      if (appPointer->GetShowRenderFrame()) {
        fpsText +=
            "Render FPS: " + std::to_string(appPointer->GetRenderFrameCount());
      }
      if (appPointer->GetShowRenderFrame() && appPointer->GetShowGameFrame()) {
        fpsText += " | ";
      }
      if (appPointer->GetShowGameFrame()) {
        fpsText +=
            "Logic FPS: " + std::to_string(appPointer->GetGameFrameCount());
      }
      ImVec2 textSize = ImGui::CalcTextSize(fpsText.c_str());
      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textSize.x) * 0.5f);
      ImGui::Text("%s", fpsText.c_str());
      ImGui::End();
    }
  }
}

void BaseEditor::EditorDrawLaunchCommand() {
  ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 30));
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 30));
  if (ImGui::Begin("Scene Control", nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoSavedSettings)) {
    float buttonWidth = 150.0f;
    float buttonHeight = 30.0f;
    ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - 161);
    ImGui::SetCursorPosY((ImGui::GetWindowSize().y - buttonHeight) * 0.5f);

    ImGui::BeginDisabled(appPointer->GetSceneState() != SceneState::Terminated);
    if (ImGui::Button("Launch Scene", ImVec2(buttonWidth, buttonHeight))) {
      appPointer->LaunchScene();
      showSceneHierarchy = true;
      expandAllObjects = true;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f + 13);
    ImGui::SetCursorPosY((ImGui::GetWindowSize().y - buttonHeight) * 0.5f);

    ImGui::BeginDisabled(appPointer->GetSceneState() != SceneState::Running);
    if (ImGui::Button("Terminate Scene", ImVec2(buttonWidth, buttonHeight))) {
      appPointer->TerminateScene();
      showSceneHierarchy = false;
    }
    ImGui::EndDisabled();
    ImGui::End();
  }
}

namespace fs = std::filesystem;
using namespace rapidjson;

std::weak_ptr<Document> BaseEditor::LoadJsonFile(const fs::path& path) {
  return JsonUtils::GetJsonDocFromFile(GetRoot() +
                                       ParseFilePath(path.string()));
}

void BaseEditor::DisplayJson(Value& value, bool& modifiedValue) {
  if (modifiedValue) {
    return;
  }
  if (value.IsObject()) {
    for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
      if (expandAllProperties) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
      } else if (collapseAllProperties) {
        ImGui::SetNextItemOpen(false, ImGuiCond_Always);
      }
      if (ImGui::TreeNode(it->name.GetString())) {
        DisplayJson(it->value, modifiedValue);
        ImGui::TreePop();
      }
    }
  } else if (value.IsArray()) {
    int index = 0;
    for (auto& item : value.GetArray()) {
      if (expandAllProperties) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
      } else if (collapseAllProperties) {
        ImGui::SetNextItemOpen(false, ImGuiCond_Always);
      }
      if (ImGui::TreeNode(("Index " + std::to_string(index++)).c_str())) {
        DisplayJson(item, modifiedValue);
        ImGui::TreePop();
      }
    }
  } else if (value.IsBool()) {
    bool val = value.GetBool();
    if (ImGui::Checkbox("##Value", &val)) {
      value.SetBool(val);
      modifiedValue = true;
      JsonUtils::WriteDocumentToFile(selectedFile.first, selectedFile.second,
                                     false);
    }
  } else if (value.IsInt()) {
    int val = value.GetInt();
    if (ImGui::InputInt("##Value", &val, 0, 0,
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
      value.SetInt(val);
      modifiedValue = true;
      JsonUtils::WriteDocumentToFile(selectedFile.first, selectedFile.second,
                                     false);
    }
  } else if (value.IsDouble()) {
    double val = value.GetDouble();
    if (ImGui::InputDouble("##Value", &val, 0, 0, "%.4f",
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
      value.SetDouble(val);
      modifiedValue = true;
      JsonUtils::WriteDocumentToFile(selectedFile.first, selectedFile.second,
                                     false);
    }
  } else if (value.IsString()) {
    std::string val = value.GetString();

    char buf[256];
    strncpy_s(buf, sizeof(buf), val.c_str(), sizeof(buf));
    buf[val.size()] = 0;

    if (ImGui::InputText("##Value", buf, sizeof(buf),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      auto selectedFilePtr = selectedFile.second.lock();
      auto& allocator = selectedFilePtr->GetAllocator();

      std::string bufStr(buf);
      value.SetString(bufStr.data(), bufStr.size(), allocator);

      modifiedValue = true;
      JsonUtils::WriteDocumentToFile(selectedFile.first, selectedFile.second,
                                     false);
    }
  }
}

void BaseEditor::DisplayDirectory(const fs::path& path) {
  for (const auto& entry : fs::directory_iterator(path)) {
    if (entry.is_directory()) {
      if (expandAllFiles) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
      } else if (collapseAllFiles) {
        ImGui::SetNextItemOpen(false, ImGuiCond_Always);
      }
      if (ImGui::TreeNode(entry.path().filename().string().c_str())) {
        DisplayDirectory(entry.path());
        ImGui::TreePop();
      }
    } else {
      if (entry.path().extension() == ".json") {
        bool selected = (entry.path().string() == selectedFile.first);
        if (ImGui::Selectable(
                (" * " + entry.path().filename().replace_extension().string())
                    .c_str(),
                selected)) {
          selectedFile.first = entry.path().string();
          selectedFile.second = LoadJsonFile(selectedFile.first);
          lastSelectType = LastSelectType::File;
          expandAllProperties = true;

          if (CheckDoubleClick()) {
            showObjectInspector = true;
          }
        }
      } else if (onlyShowEngineFiles == false) {
        ImGui::BulletText("%s", entry.path().filename().string().c_str());
      }
    }
  }
}

std::string BaseEditor::ParseFilePath(std::string path) {
  int index = -1;
  while ((index = path.find("\\")) != -1) {
    path = path.replace(index, 1, "/");
  }
  path = path.substr(path.find(GetRoot()) + GetRoot().size());
  path = path.substr(0, path.rfind('.'));
  return path;
}

std::string BaseEditor::GetObjectName(const std::string& originName,
                                      std::weak_ptr<SceneObject> self,
                                      std::weak_ptr<SceneObject> parent) {
  std::string objectName = originName;
  if (auto parentPtr = parent.lock()) {
    int nameIndex = 0;
    while (true) {
      bool alreadyHasName = false;
      for (std::shared_ptr<SceneObject> son : parentPtr->GetSons()) {
        if (self.lock() != son && objectName == son->GetName()) {
          nameIndex++;
          alreadyHasName = true;
          objectName = originName + " (" + std::to_string(nameIndex) + ")";
        }
      }
      if (alreadyHasName == false) {
        break;
      }
    }
  }
  return objectName;
}

bool BaseEditor::GetAddToSceneEnable() {
  bool addToSceneEnable = false;
  auto selectedFilePtr = selectedFile.second.lock();
  if (selectedObject.second.lock() && selectedFilePtr &&
      selectedFilePtr->HasMember("Type")) {
    auto& value = (*selectedFilePtr)["Type"];
    for (SizeType i = 0; i < value.Size(); i++) {
      if (strcmp(value[i].GetString(), "SceneObject") == 0) {
        addToSceneEnable = true;
        break;
      }
    }
  }
  return addToSceneEnable;
}

void BaseEditor::EditorDrawFileExplorer() {
  if (showFileExplorer) {
    float topWindowHeight = 0;
    float buttonWindowHeight = 30.0f;
    float menuBarHeight = ImGui::GetFrameHeight();

    if (appPointer->GetSceneState() == SceneState::Running &&
        (appPointer->GetShowRenderFrame() || appPointer->GetShowGameFrame())) {
      topWindowHeight += 32.0f;
    }
    if (GetShowRestartTip()) {
      topWindowHeight += 32.0f;
    }

    int detailsContentNum = 1;
    if (showSceneHierarchy) {
      detailsContentNum++;
    }
    if (showObjectInspector) {
      detailsContentNum++;
    }
    float fileExplorerWidth = ImGui::GetIO().DisplaySize.x / detailsContentNum;

    float titleWindowSizeY = 32.0f;
    ImVec2 windowPos = ImVec2(0, menuBarHeight + topWindowHeight);
    ImVec2 windowSize = ImVec2(fileExplorerWidth, titleWindowSizeY);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("File Explorer Title", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      const char* title = "File Explorer";
      ImVec2 textSize = ImGui::CalcTextSize(title);
      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textSize.x) * 0.5f);
      ImGui::Text("%s", title);
      ImGui::End();
    }

    windowPos = ImVec2(0, menuBarHeight + topWindowHeight + titleWindowSizeY);
    windowSize =
        ImVec2(fileExplorerWidth, ImGui::GetIO().DisplaySize.y - menuBarHeight -
                                      topWindowHeight - titleWindowSizeY -
                                      buttonWindowHeight - 70.0f);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("File Explorer", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      DisplayDirectory(fs::current_path().string() + "/" +
                       appPointer->GetRoot());
      expandAllFiles = false;
      collapseAllFiles = false;
      ImGui::End();
    }

    windowPos = ImVec2(0, ImGui::GetIO().DisplaySize.y - 100.0f);
    windowSize = ImVec2(fileExplorerWidth, 35.0f);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("File Explorer CheckBox", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      ImGui::SetCursorPosX(
          (ImGui::GetWindowSize().x -
           ImGui::CalcTextSize("Only show engine/game files").x) *
          0.5f);
      ImGui::Checkbox("Only show engine/game files", &onlyShowEngineFiles);
      ImGui::End();
    }

    windowPos = ImVec2(0, ImGui::GetIO().DisplaySize.y - 65.0f);
    windowSize = ImVec2(fileExplorerWidth, 35.0f);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("File Explorer Command", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      float buttonWidth = 120.0f;
      float buttonHeight = 20.0f;

      ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - 202.0f);
      if (ImGui::Button("Expand All", ImVec2(buttonWidth, buttonHeight))) {
        expandAllFiles = true;
      }

      ImGui::SameLine();
      ImGui::Text("|");
      ImGui::SameLine();

      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) * 0.5f);
      if (ImGui::Button("Collapse All", ImVec2(buttonWidth, buttonHeight))) {
        collapseAllFiles = true;
      }

      ImGui::SameLine();
      ImGui::Text("|");
      ImGui::SameLine();
      ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f + 82.0f);

      ImGui::BeginDisabled(!GetAddToSceneEnable());
      if (ImGui::Button("Add To Scene", ImVec2(buttonWidth, buttonHeight))) {
        auto selectedFilePtr = selectedFile.second.lock();
        std::shared_ptr<SceneObject> object;
        std::string objectName =
            GetObjectName(selectedFilePtr->HasMember("Name")
                              ? (*selectedFilePtr)["Name"].GetString()
                              : "Unset",
                          object, selectedObject.second);

        auto& value = (*selectedFilePtr)["Type"];
        for (SizeType i = 0; i < value.Size(); i++) {
          if (strcmp(value[i].GetString(), "BaseCamera") == 0) {
            object = BaseObject::CreateImmediately<BaseCamera>(
                appPointer->GetGraphics(), selectedObject.second, objectName,
                GetRoot(), ParseFilePath(selectedFile.first),
                appPointer->GetScene());
            if (selectedFilePtr->HasMember("DefaultTransform")) {
              std::static_pointer_cast<BaseCamera>(object)->InitRotation(
                  glm::radians(ParseGLMVec3(
                      (*selectedFilePtr)["DefaultTransform"]["Rotation"]
                          .GetString())));
            }
            break;
          } else if (strcmp(value[i].GetString(), "BaseModel") == 0) {
            object = BaseObject::CreateImmediately<BaseModel>(
                appPointer->GetGraphics(), selectedObject.second, objectName,
                GetRoot(), ParseFilePath(selectedFile.first),
                appPointer->GetScene());
            if (selectedFilePtr->HasMember("DefaultCamera")) {
              std::static_pointer_cast<BaseModel>(object)->SetCamera(
                  (*selectedFilePtr)["DefaultCamera"].GetString());
            }
            if (selectedFilePtr->HasMember("DefaultLightChannel")) {
              std::static_pointer_cast<BaseModel>(object)->SetLightChannel(
                  (*selectedFilePtr)["DefaultLightChannel"].GetString());
            }
            break;
          } else if (strcmp(value[i].GetString(), "SpotLight") == 0) {
            object = BaseObject::CreateImmediately<SpotLight>(
                appPointer->GetGraphics(), selectedObject.second, objectName,
                GetRoot(), ParseFilePath(selectedFile.first),
                appPointer->GetScene());
            if (auto allLightsChannel =
                    appPointer->GetLightChannelByName("All").lock()) {
              allLightsChannel->AddLightToChannel(
                  static_pointer_cast<BaseLight>(object));
            }
            break;
          } else if (strcmp(value[i].GetString(), "SunLight") == 0) {
            object = BaseObject::CreateImmediately<SunLight>(
                selectedObject.second, objectName, GetRoot(),
                ParseFilePath(selectedFile.first), appPointer->GetScene());
            if (auto allLightsChannel =
                    appPointer->GetLightChannelByName("All").lock()) {
              allLightsChannel->AddLightToChannel(
                  static_pointer_cast<BaseLight>(object));
            }
            break;
          }
        }
      }
      ImGui::EndDisabled();
      ImGui::End();
    }
  }
}

bool BaseEditor::CheckDoubleClick() {
  static double lastClickTime = 0;
  double currentTime = ImGui::GetTime();
  if (currentTime - lastClickTime < DoubleClickTimeInterval) {
    lastClickTime = currentTime;
    return true;
  } else {
    lastClickTime = currentTime;
    return false;
  }
}

void BaseEditor::DisplayHierarchy(std::weak_ptr<SceneObject> root,
                                  std::string path) {
  if (auto rootPtr = root.lock()) {
    path += "/" + rootPtr->GetName();
    if (rootPtr->GetSons().empty() == false) {
      if (expandAllObjects) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
      } else if (collapseAllObjects) {
        ImGui::SetNextItemOpen(false, ImGuiCond_Always);
      }
      if (ImGui::TreeNode(rootPtr->GetName().c_str())) {
        bool selected = (path == selectedObject.first);
        if (ImGui::Selectable((" * " + rootPtr->GetName() + " (Self)").c_str(),
                              selected)) {
          selectedObject.first = path;
          selectedObject.second = root;
          lastSelectType = LastSelectType::Object;
          expandAllProperties = true;

          if (CheckDoubleClick()) {
            showObjectInspector = true;
          }
        }
        for (std::weak_ptr<SceneObject> son : rootPtr->GetSons()) {
          DisplayHierarchy(son, path);
        }
        ImGui::TreePop();
      }
    } else {
      bool selected = (path == selectedObject.first);
      if (ImGui::Selectable((" * " + rootPtr->GetName()).c_str(), selected)) {
        selectedObject.first = path;
        selectedObject.second = root;
        lastSelectType = LastSelectType::Object;
        expandAllProperties = true;

        if (CheckDoubleClick()) {
          showObjectInspector = true;
        }
      }
    }
  }
}

void BaseEditor::EditorDrawSceneHierarchy() {
  static bool lastShowSceneHierarchy = showSceneHierarchy;
  if (lastShowSceneHierarchy != showSceneHierarchy) {
    if (showSceneHierarchy) {
      expandAllObjects = true;
    }
    lastShowSceneHierarchy = showSceneHierarchy;
  }
  if (showSceneHierarchy) {
    float topWindowHeight = 0;
    float buttonWindowHeight = 30.0f;
    float menuBarHeight = ImGui::GetFrameHeight();

    if (appPointer->GetSceneState() == SceneState::Running &&
        (appPointer->GetShowRenderFrame() || appPointer->GetShowGameFrame())) {
      topWindowHeight = 32.0f;
    }
    if (GetShowRestartTip()) {
      topWindowHeight += 32.0f;
    }

    int detailsContentNum = 1;
    if (showFileExplorer) {
      detailsContentNum++;
    }
    if (showObjectInspector) {
      detailsContentNum++;
    }
    float sceneHierarchyWidth =
        ImGui::GetIO().DisplaySize.x / detailsContentNum;

    float sceneHierarchyPosX = 0;
    if (detailsContentNum == 2 && showFileExplorer || detailsContentNum == 3) {
      sceneHierarchyPosX = sceneHierarchyWidth;
    }

    float titleWindowSizeY = 32.0f;
    ImVec2 windowPos =
        ImVec2(sceneHierarchyPosX, menuBarHeight + topWindowHeight);
    ImVec2 windowSize = ImVec2(sceneHierarchyWidth, titleWindowSizeY);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("Scene Hierarchy Title", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      const char* title = "Scene Hierarchy";
      ImVec2 textSize = ImGui::CalcTextSize(title);
      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textSize.x) * 0.5f);
      ImGui::Text("%s", title);
      ImGui::End();
    }

    windowPos = ImVec2(sceneHierarchyPosX,
                       menuBarHeight + topWindowHeight + titleWindowSizeY);
    windowSize =
        ImVec2(sceneHierarchyWidth,
               ImGui::GetIO().DisplaySize.y - menuBarHeight - topWindowHeight -
                   titleWindowSizeY - buttonWindowHeight - 35.0f);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("Scene Hierarchy", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      DisplayHierarchy(appPointer->GetSceneRootObject(), "");
      expandAllObjects = false;
      collapseAllObjects = false;
      ImGui::End();
    }

    windowPos =
        ImVec2(sceneHierarchyPosX, ImGui::GetIO().DisplaySize.y - 65.0f);
    windowSize = ImVec2(sceneHierarchyWidth, 35.0f);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("Scene Hierarchy Command", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      float buttonWidth = 120.0f;
      float buttonHeight = 20.0f;

      ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - 202.0f);
      if (ImGui::Button("Expand All", ImVec2(buttonWidth, buttonHeight))) {
        expandAllObjects = true;
      }

      ImGui::SameLine();
      ImGui::Text("|");
      ImGui::SameLine();

      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) * 0.5f);
      if (ImGui::Button("Collapse All", ImVec2(buttonWidth, buttonHeight))) {
        collapseAllObjects = true;
      }

      ImGui::SameLine();
      ImGui::Text("|");
      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f + 82.0f);
      auto object = selectedObject.second.lock();
      ImGui::BeginDisabled(object == nullptr || object->GetAlive() == false);
      if (ImGui::Button("Remove Object", ImVec2(buttonWidth, buttonHeight))) {
        object->Destroy();
      }
      ImGui::EndDisabled();
      ImGui::End();
    }
  }
}

#define ProcessExpandOrCollapse                        \
  {                                                    \
    if (expandAllProperties) {                         \
      ImGui::SetNextItemOpen(true, ImGuiCond_Always);  \
    } else if (collapseAllProperties) {                \
      ImGui::SetNextItemOpen(false, ImGuiCond_Always); \
    }                                                  \
  }
#define EditorChangeTransformEPS 0.001f
void BaseEditor::DisplayProperties(std::weak_ptr<SceneObject> object) {
  if (auto objPtr = object.lock()) {
    ProcessExpandOrCollapse;
    if (ImGui::TreeNode("Name")) {
      char buf[256];
      strncpy_s(buf, sizeof(buf), objPtr->GetName().c_str(), sizeof(buf));
      buf[objPtr->GetName().size()] = 0;

      if (ImGui::InputText("##Value", buf, sizeof(buf),
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
        objPtr->SetName(GetObjectName(buf, object, objPtr->GetParent()));
      }
      ImGui::TreePop();
    }
    ProcessExpandOrCollapse;
    if (ImGui::TreeNode("Transform")) {
      ProcessExpandOrCollapse;
      if (ImGui::TreeNode("Relative")) {
        glm::vec3 pos = objPtr->GetRelativePosition();
        float posBuf[3]{pos.x, pos.y, pos.z};
        ImGui::InputFloat3("Position", posBuf);
        if (abs(posBuf[0] - pos.x) > EditorChangeTransformEPS ||
            abs(posBuf[1] - pos.y) > EditorChangeTransformEPS ||
            abs(posBuf[2] - pos.z) > EditorChangeTransformEPS) {
          objPtr->SetRelativePosition(
              glm::vec3(posBuf[0], posBuf[1], posBuf[2]));
        }

        glm::vec3 rot = glm::degrees(objPtr->GetRelativeRotation());
        float rotBuf[3]{rot.x, rot.y, rot.z};
        ImGui::InputFloat3("Rotation", rotBuf);
        if (abs(rotBuf[0] - rot.x) > EditorChangeTransformEPS ||
            abs(rotBuf[1] - rot.y) > EditorChangeTransformEPS ||
            abs(rotBuf[2] - rot.z) > EditorChangeTransformEPS) {
          objPtr->SetRelativeRotation(
              glm::radians(glm::vec3(rotBuf[0], rotBuf[1], rotBuf[2])));
        }

        glm::vec3 sca = objPtr->GetRelativeScale();
        float scaBuf[3]{sca.x, sca.y, sca.z};
        ImGui::InputFloat3("Scale", scaBuf);
        if (abs(scaBuf[0] - sca.x) > EditorChangeTransformEPS ||
            abs(scaBuf[1] - sca.y) > EditorChangeTransformEPS ||
            abs(scaBuf[2] - sca.z) > EditorChangeTransformEPS) {
          objPtr->SetRelativeScale(glm::vec3(scaBuf[0], scaBuf[1], scaBuf[2]));
        }
        ImGui::TreePop();
      }
      ProcessExpandOrCollapse;
      if (ImGui::TreeNode("Absolute")) {
        glm::vec3 pos = objPtr->GetAbsolutePosition();
        float posBuf[3]{pos.x, pos.y, pos.z};
        ImGui::InputFloat3("Position", posBuf);
        if (abs(posBuf[0] - pos.x) > EditorChangeTransformEPS ||
            abs(posBuf[1] - pos.y) > EditorChangeTransformEPS ||
            abs(posBuf[2] - pos.z) > EditorChangeTransformEPS) {
          objPtr->SetAbsolutePosition(
              glm::vec3(posBuf[0], posBuf[1], posBuf[2]));
        }

        glm::vec3 rot = glm::degrees(objPtr->GetAbsoluteRotation());
        float rotBuf[3]{rot.x, rot.y, rot.z};
        ImGui::InputFloat3("Rotation", rotBuf);
        if (abs(rotBuf[0] - rot.x) > EditorChangeTransformEPS ||
            abs(rotBuf[1] - rot.y) > EditorChangeTransformEPS ||
            abs(rotBuf[2] - rot.z) > EditorChangeTransformEPS) {
          objPtr->SetAbsoluteRotation(
              glm::radians(glm::vec3(rotBuf[0], rotBuf[1], rotBuf[2])));
        }

        glm::vec3 sca = objPtr->GetAbsoluteScale();
        float scaBuf[3]{sca.x, sca.y, sca.z};
        ImGui::InputFloat3("Scale", scaBuf);
        if (abs(scaBuf[0] - sca.x) > EditorChangeTransformEPS ||
            abs(scaBuf[1] - sca.y) > EditorChangeTransformEPS ||
            abs(scaBuf[2] - sca.z) > EditorChangeTransformEPS) {
          objPtr->SetAbsoluteScale(glm::vec3(scaBuf[0], scaBuf[1], scaBuf[2]));
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }
    if (auto cameraPtr = dynamic_pointer_cast<BaseCamera>(objPtr)) {
    } else if (auto cameraPtr = dynamic_pointer_cast<BaseCamera>(objPtr)) {
    }
  }
}
#undef EditorChangeTransformEPS
#undef ProcessExpandOrCollapse

void BaseEditor::EditorDrawObjectInspector() {
  static bool lastShowObjectInspector = showObjectInspector;
  if (lastShowObjectInspector != showObjectInspector) {
    if (showObjectInspector) {
      expandAllProperties = true;
    }
    lastShowObjectInspector = showObjectInspector;
  }
  if (showObjectInspector) {
    float topWindowHeight = 0;
    float buttonWindowHeight = 30.0f;
    float menuBarHeight = ImGui::GetFrameHeight();

    if (appPointer->GetSceneState() == SceneState::Running &&
        (appPointer->GetShowRenderFrame() || appPointer->GetShowGameFrame())) {
      topWindowHeight = 32.0f;
    }
    if (GetShowRestartTip()) {
      topWindowHeight += 32.0f;
    }

    int detailsContentNum = 1;
    if (showFileExplorer) {
      detailsContentNum++;
    }
    if (showSceneHierarchy) {
      detailsContentNum++;
    }
    float objectInspectorWidth =
        ImGui::GetIO().DisplaySize.x / detailsContentNum;
    float objectInspectorPosX = (detailsContentNum - 1) * objectInspectorWidth;

    float titleWindowSizeY = 32.0f;
    ImVec2 windowPos =
        ImVec2(objectInspectorPosX, menuBarHeight + topWindowHeight);
    ImVec2 windowSize = ImVec2(objectInspectorWidth, titleWindowSizeY);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("Object Inspector Title", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      const char* title = "Object Inspector";
      ImVec2 textSize = ImGui::CalcTextSize(title);
      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textSize.x) * 0.5f);
      ImGui::Text("%s", title);
      ImGui::End();
    }

    windowPos = ImVec2(objectInspectorPosX,
                       menuBarHeight + topWindowHeight + titleWindowSizeY);
    windowSize =
        ImVec2(objectInspectorWidth,
               ImGui::GetIO().DisplaySize.y - menuBarHeight - topWindowHeight -
                   titleWindowSizeY - buttonWindowHeight - 35.0f);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("Object Inspector", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      auto selectedFilePtr = selectedFile.second.lock();
      if (lastSelectType == LastSelectType::File && selectedFilePtr &&
          selectedFilePtr->IsObject()) {
        bool valueModified = false;
        DisplayJson(*selectedFilePtr, valueModified);
        expandAllProperties = false;
        collapseAllProperties = false;
      } else if (lastSelectType == LastSelectType::Object &&
                 selectedObject.second.lock()) {
        DisplayProperties(selectedObject.second);
        expandAllProperties = false;
        collapseAllProperties = false;
      }
      ImGui::End();
    }

    windowPos =
        ImVec2(objectInspectorPosX, ImGui::GetIO().DisplaySize.y - 65.0f);
    windowSize = ImVec2(objectInspectorWidth, 35.0f);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    if (ImGui::Begin("Object Inspector Command", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings)) {
      float buttonWidth = 120.0f;
      float buttonHeight = 20.0f;

      ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - 202.0f);
      if (ImGui::Button("Expand All", ImVec2(buttonWidth, buttonHeight))) {
        expandAllProperties = true;
      }

      ImGui::SameLine();
      ImGui::Text("|");
      ImGui::SameLine();

      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) * 0.5f);
      if (ImGui::Button("Collapse All", ImVec2(buttonWidth, buttonHeight))) {
        collapseAllProperties = true;
      }

      ImGui::SameLine();
      ImGui::Text("|");
      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f + 82.0f);
      if (ImGui::Button("XXX", ImVec2(buttonWidth, buttonHeight))) {
      }
      ImGui::End();
    }
  }
}