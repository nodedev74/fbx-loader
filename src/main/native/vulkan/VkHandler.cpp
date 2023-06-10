#include "com_github_nodedev74_jfbx_vulkan_VkHandler.h"
#include <jni.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-pack"
#pragma clang diagnostic pop

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#include <vulkan/vulkan.hpp>

#include <fstream>
#include <vector>

VkInstance instance = VK_NULL_HANDLE;

VkPhysicalDevice gpu = VK_NULL_HANDLE;
VkDevice device = VK_NULL_HANDLE;
VkQueue graphicsQueue = VK_NULL_HANDLE;

VkSurfaceKHR surface = VK_NULL_HANDLE;
VkSurfaceFormatKHR surfaceFormat;
VkSwapchainKHR swapchain;
std::vector<VkImageView> imageViews;

VkRenderPass renderPass;
VkPipeline pipeline = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
std::vector<VkFramebuffer> framebuffers;

uint32_t graphicsQueueFamilyIndex = UINT32_MAX;

#include <iostream>

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
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_destroy(JNIEnv *env, jobject obj)
{
    vkDeviceWaitIdle(device);

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

/**
 * @brief
 *
 * @param requiredExtensions
 * @return true
 * @return false
 */
bool validateExtensions(const std::vector<const char *> &requiredExtensions)
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    for (const auto &requiredExtension : requiredExtensions)
    {
        bool extensionFound = false;
        for (const auto &availableExtension : availableExtensions)
        {
            if (strcmp(requiredExtension, availableExtension.extensionName) == 0)
            {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound)
        {
            return false;
        }
    }

    return true;
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

    const char *validationOutputFilePath = "C:/dev/usr/validation_output.txt";
    validationOutputFile.open(validationOutputFilePath);
    if (!validationOutputFile.is_open())
    {
        std::cerr << "Failed to open validation output file: " << validationOutputFilePath << std::endl;
        return;
    }

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return;
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerCreateInfo.pfnUserCallback = DebugCallback;

    uint32_t extensionCount;
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);
    std::vector<const char *> extensionNames;
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, extensionNames.data());

    extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    if (!validateExtensions(extensionNames))
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkExtensionError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        std::string rawMessage("Unsupported VkInstance extension");
        jstring message = env->NewStringUTF(rawMessage.c_str());
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    VkApplicationInfo appInfo{};

    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    instInfo.ppEnabledExtensionNames = extensionNames.data();
    instInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instInfo.ppEnabledLayerNames = validationLayers.data();

    VkResult result = vkCreateInstance(&instInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass;
        jstring message;

        switch (result)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutOfMemoryError");
            message = env->NewStringUTF("Host ran out of memory");
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutOfMemoryError");
            message = env->NewStringUTF("Device ran out of memory");
        }
        case VK_ERROR_INITIALIZATION_FAILED:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
            message = env->NewStringUTF("Failed to initialize VkInstance");
        }
        case VK_ERROR_LAYER_NOT_PRESENT:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkLayerNotPresentError");
            message = env->NewStringUTF("Required layer is not present");
        }
        case VK_ERROR_EXTENSION_NOT_PRESENT:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkExtensionError");
            message = env->NewStringUTF("Required extension not present");
        }
        case VK_ERROR_INCOMPATIBLE_DRIVER:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkDriverError");
            message = env->NewStringUTF("Incompatible driver found");
        }
        }

        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jint jResult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerEXT debugMessenger;
    vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger);

    if (!SDL_Vulkan_CreateSurface(sdlWindow, instance, &surface))
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Failed to initialize VkSurfaceKHR");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_selectPhysicalDevice(JNIEnv *env, jobject obj)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Found no supported VkPhysicalDevice");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    gpu = devices[0];

    if (gpu == VK_NULL_HANDLE)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkNullHandleError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("VkPhysicalDevice is VK_NULL_HANDLE");
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
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createDevice(JNIEnv *env, jobject obj)
{
    std::vector<const char *> deviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamilyIndex = i;
            break;
        }
    }
    if (graphicsQueueFamilyIndex == UINT32_MAX)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("No suitable Grafik-Queue-Familie found");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
    }

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensionNames.size();
    createInfo.ppEnabledExtensionNames = deviceExtensionNames.data();

    VkResult result = vkCreateDevice(gpu, &createInfo, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass;
        jstring message;

        switch (result)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Host ran out of memory");
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Device ran out of memory");
        }
        case VK_ERROR_INITIALIZATION_FAILED:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
            message = env->NewStringUTF("Failed to initialize VkDevice");
        }
        case VK_ERROR_EXTENSION_NOT_PRESENT:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Required extension not present");
        }
        case VK_ERROR_FEATURE_NOT_PRESENT:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Required feature not present");
        }
        case VK_ERROR_TOO_MANY_OBJECTS:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Too many objects");
        }
        case VK_ERROR_DEVICE_LOST:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Lost VkDevice");
        }
        }

        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jint jResult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
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

    if (surface == VK_NULL_HANDLE)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkNullHandleError");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("VkSurfaceKHR is VK_NULL_HANDLE");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, surfaceFormats.data());
    surfaceFormat = surfaceFormats[0];

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes.data());
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto &mode : presentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = mode;
            break;
        }
    }

    uint32_t desiredImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && desiredImageCount > surfaceCapabilities.maxImageCount)
    {
        desiredImageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = desiredImageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass;
        jstring message;

        switch (result)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Host ran out of memory");
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Device ran out of memory");
        }
        case VK_ERROR_INITIALIZATION_FAILED:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInitializationError");
            message = env->NewStringUTF("Failed to initialize VkSwapchainKHR");
        }
        case VK_ERROR_DEVICE_LOST:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Lost VkDevice");
        }
        case VK_ERROR_SURFACE_LOST_KHR:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Lost VkSurfaceKHR");
        }
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Native window is in use");
        }
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
            message = env->NewStringUTF("Compression exhausted extension");
        }
        }

        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jint jResult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createImageViews(JNIEnv *env, jobject obj)
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    std::vector<VkImage> swapchainImages(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    imageViews.resize(imageCount);

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = surfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]);
        if (result != VK_SUCCESS)
        {
            jclass exceptionClass;
            jstring message;

            switch (result)
            {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
            {
                exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
                message = env->NewStringUTF("Host ran out of memory");
            }
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            {
                exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
                message = env->NewStringUTF("Device ran out of memory");
            }
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:
            {
                exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkRuntimeError");
                message = env->NewStringUTF("Invalid opaque capture address");
            }
            }

            jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
            jint jResult = static_cast<jint>(result);
            jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
            env->Throw(static_cast<jthrowable>(exceptionObject));
            return;
        }
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createRenderPass(JNIEnv *env, jobject obj)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass;
        jstring message;

        switch (result)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Host ran out of memory");
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Device ran out of memory");
        }
        }

        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jint jResult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
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
    VkResult result = vkCreateShaderModule(device, &module_info, nullptr, &shader_module);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass;
        jstring message;

        switch (result)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Host ran out of memory");
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Device ran out of memory");
        }
        case VK_ERROR_INVALID_SHADER_NV:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkShaderError");
            message = env->NewStringUTF("Invalid Shader");
        }
        }
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jint jResult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return shader_module;
    }
    return shader_module;
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createGraphicsPipeline(JNIEnv *env, jobject obj)
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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities);
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
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass;
        jstring message;

        switch (result)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Host ran out of memory");
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Device ran out of memory");
        }
        }
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jint jResult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
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

    VkResult vresult = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
    if (vresult != VK_SUCCESS)
    {
        jclass exceptionClass;
        jstring message;

        switch (vresult)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Host ran out of memory");
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
            message = env->NewStringUTF("Device ran out of memory");
        }
        case VK_ERROR_INVALID_SHADER_NV:
        {
            exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkShaderError");
            message = env->NewStringUTF("Invalid shader");
        }
        }
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jint jResult = static_cast<jint>(vresult);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
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
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createFramebuffers(JNIEnv *env, jobject obj)
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

    std::vector<VkImage> swapchainImages(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);

    for (auto &image_view : imageViews)
    {
        VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fb_info.renderPass = renderPass;
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = &image_view;
        fb_info.width = surfaceCapabilities.currentExtent.width;
        fb_info.height = surfaceCapabilities.currentExtent.height;
        fb_info.layers = 1;

        VkFramebuffer framebuffer;
        VkResult result = vkCreateFramebuffer(device, &fb_info, nullptr, &framebuffer);
        if (result != VK_SUCCESS)
        {
            jclass exceptionClass;
            jstring message;

            switch (result)
            {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
            {
                exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
                message = env->NewStringUTF("Host ran out of memory");
            }
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            {
                exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkOutofMemoryError");
                message = env->NewStringUTF("Device ran out of memory");
            }
            }
            jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
            jint jResult = static_cast<jint>(result);
            jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
            env->Throw(static_cast<jthrowable>(exceptionObject));
            return;
        }
        framebuffers.push_back(framebuffer);
    }
}

VkResult acquireNextImage()
{
}

/**
 * @brief
 *
 */
void teardownFramebuffers()
{
    vkQueueWaitIdle(graphicsQueue);

    for (auto &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    framebuffers.clear();
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_render(JNIEnv *env, jobject obj)
{
    vkDeviceWaitIdle(device);
}