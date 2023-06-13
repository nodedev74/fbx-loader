/**
 * @file VkHandler.cpp
 * @author Lenard Büsing (nodedev74@gmail.com)
 * @brief Contains interaction layer with Vulkan.
 * @version 0.1
 * @date 2023-06-13
 *
 * @copyright Copyright (c) 2023 Lenard Büsing
 *
 */

#define VK_USE_PLATFORM_WIN32_KHR
#define VOLK_IMPLEMENTATION

#include "com_github_nodedev74_jfbx_vulkan_VkHandler.h"
#include <jni.h>

#include "vulkan/VkHelper.hpp"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include <glm/glm.hpp>

#include "volk.h"

#include <fstream>
#include <chrono>
#include <iostream>
#include <vector>
#include <string>

using namespace VkHelper;

VkExtent2D windowSize{};

VkInstance instance;

VkSurfaceKHR surface;
VkSurfaceFormatKHR surfaceFormat;

VkPhysicalDevice physicalDevice;
VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
uint32_t queueFamilyIndex;
VkDevice device;
VkQueue queue;

VkSwapchainCreateInfoKHR swapchainCreateInfo;
VkSwapchainKHR swapchain;
uint32_t swapchainImagesCount;
std::vector<VkImage> swapchainImages;

VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;

VkBuffer hostVertexBuffer;
VkBuffer hostMatrixBuffer;
VkMemoryRequirements hostMemoryRequirements[2];
VkDeviceMemory hostMemory;
void *hostDataPointer;
VkBuffer deviceVertexBuffer;
VkBuffer deviceMatrixBuffer;
VkMemoryRequirements deviceMemoryRequirements[2];
VkDeviceMemory deviceMemory;

VkDescriptorPool descriptorPool;
VkDescriptorSetLayout descriptorSetLayout;
VkDescriptorSet descriptorSet;

VkRenderPass renderPass;
std::vector<VkFramebuffer> framebuffers;
std::vector<VkImageView> swapchainImagesViews;

VkPipelineLayout pipelineLayout;
VkPipeline pipeline;

std::vector<VkSemaphore> semaphores;
std::vector<glm::vec3> inputData = {{-0.2f, -0.2f, 0.5f}, {0.5f, 0.8f, 0.72f}, {0.2f, -0.2f, 0.5f}, {0.0f, 0.3f, 0.1f}, {0.0f, 0.2f, 0.5f}, {0.4f, 0.1f, 0.8f}};

/**
 * @brief Creates a Vulkan instance.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
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

    VkResult volkInitResult = volkInitialize();
    if (volkInitResult != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        std::string rawMessage("Failed to initialize Volk-Loader");
        jstring message = env->NewStringUTF(rawMessage.c_str());
        jint jresult = static_cast<jint>(volkInitResult);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
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

    VkResult instResult = vkCreateInstance(&instInfo, nullptr, &instance);
    if (instResult != VK_SUCCESS)
    {
        instance = VK_NULL_HANDLE;

        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        std::string rawMessage("Failed to initialize VkInstance");
        jstring message = env->NewStringUTF(rawMessage.c_str());
        jint jresult = static_cast<jint>(instResult);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    volkLoadInstance(instance);
}

/**
 * @brief Creates a Vulkan debugger.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
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
 * @brief Creates a Vulkan surface for SDLWindow.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createSureface(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, fieldID);
    SDL_Window *sdlWindow = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    if (!SDL_Vulkan_CreateSurface(sdlWindow, instance, &surface))
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkSurfaceKHR");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief Creates a logical Vulkan device.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createLogicalDevice(JNIEnv *env, jobject obj)
{
    uint32_t devicesNumber;
    vkEnumeratePhysicalDevices(instance, &devicesNumber, nullptr);
    std::vector<VkPhysicalDevice> devices(devicesNumber);
    std::vector<VkPhysicalDeviceProperties> devicesProperties(devicesNumber);
    std::vector<VkPhysicalDeviceFeatures> devicesFeatures(devicesNumber);
    vkEnumeratePhysicalDevices(instance, &devicesNumber, devices.data());

    for (uint32_t i = 0; i < devices.size(); i++)
    {
        vkGetPhysicalDeviceProperties(devices[i], &devicesProperties[i]);
        vkGetPhysicalDeviceFeatures(devices[i], &devicesFeatures[i]);
    }

    size_t selectedDeviceNumber = 0;

    physicalDevice = devices[selectedDeviceNumber];
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

    uint32_t familiesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familiesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamiliesProperties(familiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familiesCount, queueFamiliesProperties.data());

    queueFamilyIndex = -1;
    VkBool32 doesQueueFamilySupportSurface = VK_FALSE;
    while (doesQueueFamilySupportSurface == VK_FALSE)
    {
        queueFamilyIndex++;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &doesQueueFamilySupportSurface);
    }

    std::vector<float> queuePriorities = {1.0f};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
    queueCreateInfo.push_back(
        {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         nullptr,
         0,
         static_cast<uint32_t>(queueFamilyIndex),
         static_cast<uint32_t>(queuePriorities.size()),
         queuePriorities.data()});

    std::vector<const char *> desiredDeviceLevelExtensions = {"VK_KHR_swapchain"};
    VkPhysicalDeviceFeatures selectedDeviceFeatures = {0};

    VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(queueCreateInfo.size()),
        queueCreateInfo.data(),
        0,
        nullptr,
        static_cast<uint32_t>(desiredDeviceLevelExtensions.size()),
        desiredDeviceLevelExtensions.data(),
        &selectedDeviceFeatures,
    };

    VkResult deviceResult = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (deviceResult != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkDevice");
        jint jresult = static_cast<jint>(deviceResult);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    volkLoadDevice(device);
}

/**
 * @brief Creates a Vulkan swapchain.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createSwapchain(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(cls, "sdlWindowPtr", "J");
    jlong sdlWindowPtr = env->GetLongField(obj, fieldID);
    SDL_Window *sdlWindow = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    int width, height;
    SDL_GetWindowSize(sdlWindow, &width, &height);
    windowSize.width = width;
    windowSize.height = height;

    uint32_t presentationModesNumber;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentationModesNumber, nullptr);
    std::vector<VkPresentModeKHR> presentationModes(presentationModesNumber);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentationModesNumber, presentationModes.data());
    VkPresentModeKHR selectedPresentMode = VkHelper::selectPresentationMode(presentationModes, VK_PRESENT_MODE_MAILBOX_KHR);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    uint32_t numberOfImages = VkHelper::selectNumberOfImages(surfaceCapabilities);
    VkExtent2D sizeOfImages = VkHelper::selectSizeOfImages(surfaceCapabilities, windowSize);
    VkImageUsageFlags imageUsage = VkHelper::selectImageUsage(surfaceCapabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkSurfaceTransformFlagBitsKHR surfaceTransform = VkHelper::selectSurfaceTransform(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, surfaceFormats.data());
    VkSurfaceFormatKHR surfaceFormat = VkHelper::selectSurfaceFormat(surfaceFormats, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});

    swapchainCreateInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface,
        numberOfImages,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        sizeOfImages,
        1,
        imageUsage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        surfaceTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        selectedPresentMode,
        VK_TRUE,
        nullptr,
    };

    VkResult swapchainResult = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    if (swapchainResult != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkSwapchainKHR");
        jint jresult = static_cast<jint>(swapchainResult);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, nullptr);
    swapchainImages.resize(swapchainImagesCount);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, swapchainImages.data());
}

/**
 * @brief Creates a Vulkan command pool.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createCommandPool(JNIEnv *env, jobject obj)
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        0,
        queueFamilyIndex,
    };

    VkResult result = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkCommandPool");
        jint jresult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief Allocates the command buffers.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_allocateCommandBuffers(JNIEnv *env, jobject obj)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        swapchainImagesCount,
    };

    commandBuffers.resize(swapchainImagesCount);

    VkResult result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data());
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkCommandBuffer");
        jint jresult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief Creates host buffers.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createHostBuffers(JNIEnv *env, jobject obj)
{
    VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        inputData.size() * sizeof(decltype(inputData[0])),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
    };

    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &hostVertexBuffer);

    bufferCreateInfo.size = sizeof(glm::mat4);
    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &hostMatrixBuffer);

    vkGetBufferMemoryRequirements(device, hostVertexBuffer, &hostMemoryRequirements[0]);
    vkGetBufferMemoryRequirements(device, hostMatrixBuffer, &hostMemoryRequirements[1]);

    VkMemoryAllocateInfo memoryAllocateInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        hostMemoryRequirements[0].size + hostMemoryRequirements[1].size,
        VkHelper::selectMemoryIndex(physicalDeviceMemoryProperties, hostMemoryRequirements[0], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
    };

    VkResult result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &hostMemory);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to allocate memory");
        jint jresult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }

    vkBindBufferMemory(device, hostVertexBuffer, hostMemory, 0);
    vkBindBufferMemory(device, hostMatrixBuffer, hostMemory, hostMemoryRequirements[0].size);

    vkMapMemory(device, hostMemory, 0, VK_WHOLE_SIZE, 0, &hostDataPointer);
    memcpy(hostDataPointer, inputData.data(), inputData.size() * sizeof(decltype(inputData[0])));
    VkMappedMemoryRange mapped_memory_range = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, hostMemory, 0, VK_WHOLE_SIZE};
    vkFlushMappedMemoryRanges(device, 1, &mapped_memory_range);
}

/**
 * @brief Creates device buffers.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createDeviceBuffers(JNIEnv *env, jobject obj)
{
    VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        inputData.size() * sizeof(decltype(inputData[0])),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
    };

    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &deviceVertexBuffer);

    bufferCreateInfo.size = sizeof(glm::mat4);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &deviceMatrixBuffer);

    vkGetBufferMemoryRequirements(device, deviceMatrixBuffer, &deviceMemoryRequirements[0]);
    vkGetBufferMemoryRequirements(device, deviceVertexBuffer, &deviceMemoryRequirements[1]);

    VkMemoryAllocateInfo memoryAllocateInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        deviceMemoryRequirements[0].size + deviceMemoryRequirements[1].size,
        VkHelper::selectMemoryIndex(physicalDeviceMemoryProperties, deviceMemoryRequirements[0], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    VkResult result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to allocate memory");
        jint jresult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }

    vkBindBufferMemory(device, deviceVertexBuffer, deviceMemory, 0);
    vkBindBufferMemory(device, deviceMatrixBuffer, deviceMemory, deviceMemoryRequirements[0].size);
}

/**
 * @brief Creates Descriptor pool
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createDescriptorPool(JNIEnv *env, jobject obj)
{
    VkDescriptorPoolSize descriptorPoolSize = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        0,
        1,
        1,
        &descriptorPoolSize,
    };

    vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
}

/**
 * @brief Allocates Descriptor Sets.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_allocateDescriptorSets(JNIEnv *env, jobject obj)
{
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr,
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &descriptorSetLayoutBinding,
    };

    vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        descriptorPool,
        1,
        &descriptorSetLayout,
    };

    vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);

    VkDescriptorBufferInfo descriptorBufferInfo = {
        deviceMatrixBuffer,
        0,
        sizeof(glm::mat4),
    };

    VkWriteDescriptorSet writeDescriptorSet = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr,
        &descriptorBufferInfo,
        nullptr,
    };

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

/**
 * @brief Creates a Vulkan Renderpass.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createRenderpass(JNIEnv *env, jobject obj)
{
    VkAttachmentDescription attachmentDescription = {
        0,
        swapchainCreateInfo.imageFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference attachmentReference = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpassDescription = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &attachmentReference,
        nullptr,
        nullptr,
        0,
        nullptr,
    };

    VkRenderPassCreateInfo renderPassCreateInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        1,
        &attachmentDescription,
        1,
        &subpassDescription,
        0,
        nullptr,
    };

    vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
}

/**
 * @brief Creates Vulkan Framebuffers.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createFramebuffers(JNIEnv *env, jobject obj)
{
    framebuffers.resize(swapchainImagesCount);
    swapchainImagesViews.resize(swapchainImagesCount);

    for (int i = 0; i < swapchainImagesCount; i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            swapchainImages[i],
            VK_IMAGE_VIEW_TYPE_2D,
            swapchainCreateInfo.imageFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS},
        };

        vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImagesViews[i]);

        VkFramebufferCreateInfo framebufferCreateInfo = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            renderPass,
            1,
            &swapchainImagesViews[i],
            swapchainCreateInfo.imageExtent.width,
            swapchainCreateInfo.imageExtent.height,
            swapchainCreateInfo.imageArrayLayers,
        };

        vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]);
    }
}

/**
 * @brief Reads SPIRV files.
 *
 * @param filePath Path to the shader file (temporary file).
 * @return The files content.
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
 * @brief Loads shader modules.
 *
 * @param path Internal path to the shader file.
 * @param env The JNI environment.
 * @param obj The Java object instance.
 * @return VkShaderModule The shader module to be loaded from graphics pipeline.
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

    VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    moduleInfo.codeSize = spirv.size() * sizeof(uint32_t);
    moduleInfo.pCode = spirv.data();

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(device, &moduleInfo, nullptr, &shaderModule);

    return shaderModule;
}

/**
 * @brief Creates a Vulkan graphics pipeline.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
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
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initializate VkPipelineLayout");
        jint jresult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
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
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    VkResult a_result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
    if (a_result != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initializate VkPipeline");
        jint jresult = static_cast<jint>(a_result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    vkDestroyShaderModule(device, vertShader, nullptr);
    vkDestroyShaderModule(device, fragShader, nullptr);
}

/**
 * @brief Uploads the input data.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_uploadInputData(JNIEnv *env, jobject obj)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr,
    };

    vkBeginCommandBuffer(commandBuffers[0], &commandBufferBeginInfo);
    VkBufferCopy bufferCopy = {0, 0, inputData.size() * sizeof(decltype(inputData[0]))};
    vkCmdCopyBuffer(commandBuffers[0], hostVertexBuffer, deviceVertexBuffer, 1, &bufferCopy);
    vkEndCommandBuffer(commandBuffers[0]);

    VkFenceCreateInfo fenceCreateInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        0,
    };

    VkFence fence;
    vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);

    VkPipelineStageFlags pipelineStageFlags = {VK_PIPELINE_STAGE_TRANSFER_BIT};
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        &pipelineStageFlags,
        1,
        &commandBuffers[0],
        0,
        nullptr,
    };

    vkQueueSubmit(queue, 1, &submitInfo, fence);

    vkWaitForFences(device, 1, &fence, VK_TRUE, 20000000);
    vkResetCommandPool(device, commandPool, 0);
    vkDestroyFence(device, fence, nullptr);
}

/**
 * @brief Records the command buffers for Vulkan rendering.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_recordCommandBuffers(JNIEnv *env, jobject obj)
{
    VkClearValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};

    for (int i = 0; i < swapchainImagesCount; i++)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
        vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);

        VkBufferCopy bufferCopy = {0, 0, sizeof(glm::mat4)};
        vkCmdCopyBuffer(commandBuffers[i], hostMatrixBuffer, deviceMatrixBuffer, 1, &bufferCopy);

        VkMemoryBarrier memoryBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT};
        vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

        VkImageMemoryBarrier imageMemoryBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            swapchainImages[i],
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        VkRenderPassBeginInfo renderPassBeginInfo = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            renderPass,
            framebuffers[i],
            {{0, 0}, {swapchainCreateInfo.imageExtent}},
            1,
            &clearColor};
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &deviceVertexBuffer, &offset);

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        vkEndCommandBuffer(commandBuffers[i]);
    }
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

/**
 * @brief Creates semaphores for Vulkan synchronization.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createSemaphores(JNIEnv *env, jobject obj)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    semaphores.resize(2);
    for (int i = 0; i < semaphores.size(); i++)
    {
        vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores[i]);
    }
}

/**
 * @brief Renders the Vulkan scene.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_render(JNIEnv *env, jobject obj)
{

    jclass cls = env->GetObjectClass(obj);

    uint32_t imageIndex = 0;
    VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores[0], VK_NULL_HANDLE, &imageIndex);
    if (res != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("unknown");
        jint jresult = static_cast<jint>(res);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }

    VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1,
        &semaphores[0],
        &pipelineStageFlags,
        1,
        &commandBuffers[imageIndex],
        1,
        &semaphores[1]};
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &semaphores[1],
        1,
        &swapchain,
        &imageIndex};

    res = vkQueuePresentKHR(queue, &presentInfo);
    if (res != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("unknown");
        jint jresult = static_cast<jint>(res);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jresult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }
}

/**
 * @brief Destroys the Vulkan resources.
 *
 * @param env The JNI environment.
 * @param obj The Java object instance.
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
        vkDestroyImageView(device, swapchainImagesViews[i], nullptr);
    }
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    for (int i = 0; i < semaphores.size(); i++)
    {
        vkDestroySemaphore(device, semaphores[i], nullptr);
    }
    vkUnmapMemory(device, hostMemory);
    vkDestroyBuffer(device, hostVertexBuffer, nullptr);
    vkDestroyBuffer(device, hostMatrixBuffer, nullptr);
    vkFreeMemory(device, hostMemory, nullptr);
    vkDestroyBuffer(device, deviceVertexBuffer, nullptr);
    vkDestroyBuffer(device, deviceMatrixBuffer, nullptr);
    vkFreeMemory(device, deviceMemory, nullptr);
    vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    SDL_DestroyWindow(sdlWindow);
    vkDestroyInstance(instance, nullptr);
}
