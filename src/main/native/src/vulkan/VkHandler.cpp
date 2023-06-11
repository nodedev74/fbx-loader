#define VK_USE_PLATFORM_WIN32_KHR
#define VOLK_IMPLEMENTATION

#include "com_github_nodedev74_jfbx_vulkan_VkHandler.h"
#include <jni.h>

#include "vulkan/VkHelper.hpp"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "volk.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <string>

VkExtent2D window_size;

VkInstance instance;

VkSurfaceKHR surface;
VkSurfaceFormatKHR surfaceFormat;

VkPhysicalDevice physical_device;
VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
uint32_t queue_family_index;
VkDevice device;
VkQueue queue;

VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
VkSwapchainCreateInfoKHR swapchain_create_info;
VkSwapchainKHR swapchain;
uint32_t swapchain_images_count;
std::vector<VkImage> swapchain_images;

VkCommandPool command_pool;
std::vector<VkCommandBuffer> command_buffers;

VkBuffer host_vertex_buffer;
VkBuffer host_m_matrix_buffer;
VkMemoryRequirements host_memory_requirements[2];
VkDeviceMemory host_memory;
void *host_data_pointer;
VkBuffer device_vertex_buffer;
VkBuffer device_m_matrix_buffer;
VkMemoryRequirements device_memory_requirements[2];
VkDeviceMemory device_memory;

VkDescriptorPool descriptor_pool;
VkDescriptorSetLayout descriptor_set_layout;
VkDescriptorSet descriptor_set;

VkRenderPass render_pass;
std::vector<VkFramebuffer> framebuffers;
std::vector<VkImageView> swapchain_images_views;

VkPipelineLayout pipeline_layout;
VkPipeline pipeline;

std::vector<VkSemaphore> semaphores;
std::vector<glm::vec3> input_data = {{-0.2f, -0.2f, 0.5f}, {0.5f, 0.8f, 0.72f}, {0.2f, -0.2f, 0.5f}, {0.0f, 0.3f, 0.1f}, {0.0f, 0.2f, 0.5f}, {0.4f, 0.1f, 0.8f}};

std::ofstream validationOutputFile;

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    validationOutputFile << pCallbackData->pMessage << std::endl;

    std::cout << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createInstance(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, fieldID);
    SDL_Window *sdlWindow = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    const char *validationOutputFilePath = "validation_output.txt";
    validationOutputFile.open(validationOutputFilePath);
    if (!validationOutputFile.is_open())
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        std::string rawMessage = std::string("Failed to open validation output file: ") + std::string(validationOutputFilePath);
        jstring message = env->NewStringUTF(rawMessage.c_str());
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    if (volkInitialize() != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        std::string rawMessage("Failed to initialize Volk-Loader");
        jstring message = env->NewStringUTF(rawMessage.c_str());
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    VkApplicationInfo appInfo = {};

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    uint32_t extensionCount;
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);
    std::vector<const char *> extensionNames;
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, extensionNames.data());

    extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    instInfo.ppEnabledExtensionNames = extensionNames.data();
    instInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instInfo.ppEnabledLayerNames = validationLayers.data();

    if (vkCreateInstance(&instInfo, nullptr, &instance))
    {
        instance = VK_NULL_HANDLE;

        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        std::string rawMessage("Failed to initialize VkInstance");
        jstring message = env->NewStringUTF(rawMessage.c_str());
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    volkLoadInstance(instance);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createDebugger(JNIEnv *env, jobject obj)
{
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerCreateInfo.pfnUserCallback = DebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerEXT debugMessenger;
    vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createSureface(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, fieldID);
    SDL_Window *sdlWindow = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    if (!SDL_Vulkan_CreateSurface(sdlWindow, instance, &surface))
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkSurfaceKHR");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createLogicalDevice(JNIEnv *env, jobject obj)
{
    uint32_t devices_number;
    vkEnumeratePhysicalDevices(instance, &devices_number, nullptr);
    std::vector<VkPhysicalDevice> devices(devices_number);
    std::vector<VkPhysicalDeviceProperties> devices_properties(devices_number);
    std::vector<VkPhysicalDeviceFeatures> devices_features(devices_number);
    vkEnumeratePhysicalDevices(instance, &devices_number, devices.data());

    for (uint32_t i = 0; i < devices.size(); i++)
    {
        vkGetPhysicalDeviceProperties(devices[i], &devices_properties[i]);
        vkGetPhysicalDeviceFeatures(devices[i], &devices_features[i]);
    }

    size_t selected_device_number = 0;

    physical_device = devices[selected_device_number];
    vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

    uint32_t families_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &families_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families_properties(families_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &families_count, queue_families_properties.data());

    queue_family_index = -1;
    VkBool32 does_queue_family_support_surface = VK_FALSE;
    while (does_queue_family_support_surface == VK_FALSE)
    {
        queue_family_index++;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface, &does_queue_family_support_surface);
    }

    std::vector<float> queue_priorities = {1.0f};
    std::vector<VkDeviceQueueCreateInfo> queue_create_info;
    queue_create_info.push_back({VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                 nullptr,
                                 0,
                                 static_cast<uint32_t>(queue_family_index),
                                 static_cast<uint32_t>(queue_priorities.size()),
                                 queue_priorities.data()});

    std::vector<const char *> desired_device_level_extensions = {"VK_KHR_swapchain"};
    VkPhysicalDeviceFeatures selected_device_features = {0};

    VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(queue_create_info.size()),
        queue_create_info.data(),
        0,
        nullptr,
        static_cast<uint32_t>(desired_device_level_extensions.size()),
        desired_device_level_extensions.data(),
        &selected_device_features};

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device))
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkDevice");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    vkGetDeviceQueue(device, queue_family_index, 0, &queue);
    volkLoadDevice(device);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createSwapchain(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, fieldID);
    SDL_Window *sdlWindow = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    int w, h;
    SDL_GetWindowSize(sdlWindow, &w, &h);

    window_size = {};
    window_size.width = w;
    window_size.height = h;

    uint32_t presentation_modes_number;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_modes_number, nullptr);
    std::vector<VkPresentModeKHR> presentation_modes(presentation_modes_number);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_modes_number, presentation_modes.data());
    VkPresentModeKHR selected_present_mode = VkHelper::selectPresentationMode(presentation_modes, VK_PRESENT_MODE_MAILBOX_KHR);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    uint32_t number_of_images = VkHelper::selectNumberOfImages(surface_capabilities);
    VkExtent2D size_of_images = VkHelper::selectSizeOfImages(surface_capabilities, window_size);
    VkImageUsageFlags image_usage = VkHelper::selectImageUsage(surface_capabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkSurfaceTransformFlagBitsKHR surface_transform = VkHelper::selectSurfaceTransform(surface_capabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    uint32_t formats_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formats_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formats_count, surface_formats.data());
    VkSurfaceFormatKHR surface_format = VkHelper::selectSurfaceFormat(surface_formats, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});

    swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface,
        number_of_images,
        surface_format.format,
        surface_format.colorSpace,
        size_of_images,
        1,
        image_usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        surface_transform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        selected_present_mode,
        VK_TRUE,
        old_swapchain};

    if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain) != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkSwapchainKHR");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, nullptr);
    swapchain_images.resize(swapchain_images_count);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, swapchain_images.data());
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createCommandPool(JNIEnv *env, jobject obj)
{
    VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        0,
        queue_family_index,
    };

    if (vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool))
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkCommandPool");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_allocateCommandBuffers(JNIEnv *env, jobject obj)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        swapchain_images_count,
    };

    command_buffers.resize(swapchain_images_count);

    if (vkAllocateCommandBuffers(device, &command_buffer_allocate_info, command_buffers.data()))
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkCommandBuffer");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createHostBuffers(JNIEnv *env, jobject obj)
{
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        input_data.size() * sizeof(decltype(input_data[0])),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr};
    vkCreateBuffer(device, &buffer_create_info, nullptr, &host_vertex_buffer);

    buffer_create_info.size = sizeof(glm::mat4);
    vkCreateBuffer(device, &buffer_create_info, nullptr, &host_m_matrix_buffer);

    vkGetBufferMemoryRequirements(device, host_vertex_buffer, &host_memory_requirements[0]);
    vkGetBufferMemoryRequirements(device, host_m_matrix_buffer, &host_memory_requirements[1]);

    VkMemoryAllocateInfo memory_allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        host_memory_requirements[0].size + host_memory_requirements[1].size,
        VkHelper::selectMemoryIndex(physical_device_memory_properties, host_memory_requirements[0], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)};
    if (vkAllocateMemory(device, &memory_allocate_info, nullptr, &host_memory) != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to allocate memory");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }

    vkBindBufferMemory(device, host_vertex_buffer, host_memory, 0);
    vkBindBufferMemory(device, host_m_matrix_buffer, host_memory, host_memory_requirements[0].size);

    vkMapMemory(device, host_memory, 0, VK_WHOLE_SIZE, 0, &host_data_pointer);
    memcpy(host_data_pointer, input_data.data(), input_data.size() * sizeof(decltype(input_data[0])));
    VkMappedMemoryRange mapped_memory_range = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, host_memory, 0, VK_WHOLE_SIZE};
    vkFlushMappedMemoryRanges(device, 1, &mapped_memory_range);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createDeviceBuffers(JNIEnv *env, jobject obj)
{
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        input_data.size() * sizeof(decltype(input_data[0])),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr};
    vkCreateBuffer(device, &buffer_create_info, nullptr, &device_vertex_buffer);

    buffer_create_info.size = sizeof(glm::mat4);
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkCreateBuffer(device, &buffer_create_info, nullptr, &device_m_matrix_buffer);

    vkGetBufferMemoryRequirements(device, device_m_matrix_buffer, &device_memory_requirements[0]);
    vkGetBufferMemoryRequirements(device, device_vertex_buffer, &device_memory_requirements[1]);

    VkMemoryAllocateInfo memory_allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        device_memory_requirements[0].size + device_memory_requirements[1].size,
        VkHelper::selectMemoryIndex(physical_device_memory_properties, device_memory_requirements[0], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
    if (vkAllocateMemory(device, &memory_allocate_info, nullptr, &device_memory) != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to allocate memory");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }

    vkBindBufferMemory(device, device_vertex_buffer, device_memory, 0);
    vkBindBufferMemory(device, device_m_matrix_buffer, device_memory, device_memory_requirements[0].size);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createDescriptorPool(JNIEnv *env, jobject obj)
{
    VkDescriptorPoolSize descriptor_pool_size = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1};
    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        0,
        1,
        1,
        &descriptor_pool_size};
    vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &descriptor_pool);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_allocateDescriptorSets(JNIEnv *env, jobject obj)
{
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr};
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &descriptor_set_layout_binding};
    vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        descriptor_pool,
        1,
        &descriptor_set_layout};
    vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &descriptor_set);

    VkDescriptorBufferInfo descriptor_buffer_info = {device_m_matrix_buffer, 0, sizeof(glm::mat4)};
    VkWriteDescriptorSet write_descriptor_set = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptor_set,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr,
        &descriptor_buffer_info,
        nullptr};
    vkUpdateDescriptorSets(device, 1, &write_descriptor_set, 0, nullptr);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createRenderpass(JNIEnv *env, jobject obj)
{
    VkAttachmentDescription attachment_description = {
        0,
        swapchain_create_info.imageFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    VkAttachmentReference attachment_reference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass_description = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &attachment_reference,
        nullptr,
        nullptr,
        0,
        nullptr};

    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        1,
        &attachment_description,
        1,
        &subpass_description,
        0,
        nullptr};
    vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createFramebuffers(JNIEnv *env, jobject obj)
{
    framebuffers.resize(swapchain_images_count);
    swapchain_images_views.resize(swapchain_images_count);

    for (int i = 0; i < swapchain_images_count; i++)
    {
        VkImageViewCreateInfo image_view_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            swapchain_images[i],
            VK_IMAGE_VIEW_TYPE_2D,
            swapchain_create_info.imageFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}};
        vkCreateImageView(device, &image_view_create_info, nullptr, &swapchain_images_views[i]);

        VkFramebufferCreateInfo framebuffer_create_info = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            render_pass,
            1,
            &swapchain_images_views[i],
            swapchain_create_info.imageExtent.width,
            swapchain_create_info.imageExtent.height,
            swapchain_create_info.imageArrayLayers};
        vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &framebuffers[i]);
    }
}

/**
 * @brief
 *
 * @param filePath
 * @return std::vector<uint32_t>
 */
std::vector<uint32_t> readSPIRVFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint32_t> spirvCode(fileSize / sizeof(uint32_t));
    file.read(reinterpret_cast<char *>(spirvCode.data()), fileSize);

    return spirvCode;
}

/**
 * @brief
 *
 * @param path
 * @return VkShaderModule
 */
VkShaderModule loadShaderModule(const char *name, JNIEnv *env, jobject obj)
{
    jclass cls = env->FindClass("com/github/nodedev74/jfbx/ShaderLoader");
    jmethodID methodID = env->GetStaticMethodID(cls, "load", "(Ljava/lang/String;)Ljava/lang/String;");
    jstring shaderName = env->NewStringUTF(name);
    jstring newPath = (jstring)env->CallStaticObjectMethod(cls, methodID, shaderName);

    const char *nativeString = env->GetStringUTFChars(newPath, nullptr);
    const std::string spath(nativeString);
    std::vector<uint32_t> spirv = readSPIRVFile(spath);

    VkShaderModuleCreateInfo module_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    module_info.codeSize = spirv.size() * sizeof(uint32_t);
    module_info.pCode = spirv.data();

    VkShaderModule shader_module = VK_NULL_HANDLE;
    vkCreateShaderModule(device, &module_info, nullptr, &shader_module);

    return shader_module;
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createPipeline(JNIEnv *env, jobject obj)
{
    VkShaderModule vertShader = loadShaderModule("vert", env, obj);
    VkShaderModule fragShader = loadShaderModule("frag", env, obj);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertShader;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragShader;
    fragmentShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragmentShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
    VkExtent2D swapchainExtent = capabilities.currentExtent;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initializate VkPipelineLayout");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.renderPass = render_pass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initializate VkPipeline");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    vkDestroyShaderModule(device, vertShader, nullptr);
    vkDestroyShaderModule(device, fragShader, nullptr);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_uploadInputData(JNIEnv *env, jobject obj)
{
    VkCommandBufferBeginInfo command_buffer_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
    vkBeginCommandBuffer(command_buffers[0], &command_buffer_begin_info);
    VkBufferCopy buffer_copy = {0, 0, input_data.size() * sizeof(decltype(input_data[0]))};
    vkCmdCopyBuffer(command_buffers[0], host_vertex_buffer, device_vertex_buffer, 1, &buffer_copy);
    vkEndCommandBuffer(command_buffers[0]);

    VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence fence;
    vkCreateFence(device, &fence_create_info, nullptr, &fence);

    VkPipelineStageFlags pipeline_stage_flags = {VK_PIPELINE_STAGE_TRANSFER_BIT};
    VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        &pipeline_stage_flags,
        1,
        &command_buffers[0],
        0,
        nullptr};
    vkQueueSubmit(queue, 1, &submit_info, fence);

    vkWaitForFences(device, 1, &fence, VK_TRUE, 20000000);
    vkResetCommandPool(device, command_pool, 0);
    vkDestroyFence(device, fence, nullptr);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_recordCommandBuffers(JNIEnv *env, jobject obj)
{
    VkClearValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};

    for (int i = 0; i < swapchain_images_count; i++)
    {
        VkCommandBufferBeginInfo command_buffer_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
        vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin_info);

        VkBufferCopy buffer_copy = {0, 0, sizeof(glm::mat4)};
        vkCmdCopyBuffer(command_buffers[i], host_m_matrix_buffer, device_m_matrix_buffer, 1, &buffer_copy);

        VkMemoryBarrier memory_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT};
        vkCmdPipelineBarrier(command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);

        VkImageMemoryBarrier image_memory_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            swapchain_images[i],
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        vkCmdPipelineBarrier(command_buffers[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        VkRenderPassBeginInfo render_pass_begin_info = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            render_pass,
            framebuffers[i],
            {{0, 0}, {swapchain_create_info.imageExtent}},
            1,
            &clearColor};
        vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

        vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &device_vertex_buffer, &offset);

        vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(command_buffers[i]);

        vkEndCommandBuffer(command_buffers[i]);
    }
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createSemaphores(JNIEnv *env, jobject obj)
{
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    semaphores.resize(2);
    for (int i = 0; i < semaphores.size(); i++)
    {
        vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphores[i]);
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_render(JNIEnv *env, jobject obj)
{

    jclass cls = env->GetObjectClass(obj);

    uint32_t image_index = 0;
    VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores[0], VK_NULL_HANDLE, &image_index);
    if (res != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("unknown");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }

    VkPipelineStageFlags pipeline_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1,
        &semaphores[0],
        &pipeline_stage_flags,
        1,
        &command_buffers[image_index],
        1,
        &semaphores[1]};
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

    VkPresentInfoKHR present_info = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &semaphores[1],
        1,
        &swapchain,
        &image_index};
    res = vkQueuePresentKHR(queue, &present_info);
    if (res != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("unknown");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_destroy(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, fieldID);
    SDL_Window *sdlWindow = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    vkDeviceWaitIdle(device);
    vkDestroyPipeline(device, pipeline, nullptr);
    for (int i = 0; i < framebuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, swapchain_images_views[i], nullptr);
    }
    vkDestroyRenderPass(device, render_pass, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
    for (int i = 0; i < semaphores.size(); i++)
    {
        vkDestroySemaphore(device, semaphores[i], nullptr);
    }
    vkUnmapMemory(device, host_memory);
    vkDestroyBuffer(device, host_vertex_buffer, nullptr);
    vkDestroyBuffer(device, host_m_matrix_buffer, nullptr);
    vkFreeMemory(device, host_memory, nullptr);
    vkDestroyBuffer(device, device_vertex_buffer, nullptr);
    vkDestroyBuffer(device, device_m_matrix_buffer, nullptr);
    vkFreeMemory(device, device_memory, nullptr);
    vkFreeCommandBuffers(device, command_pool, command_buffers.size(), command_buffers.data());
    vkDestroyCommandPool(device, command_pool, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    SDL_DestroyWindow(sdlWindow);
    vkDestroyInstance(instance, nullptr);
}