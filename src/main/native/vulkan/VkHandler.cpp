#include "com_github_nodedev74_jfbx_vulkan_VkHandler.h"
#include <jni.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.hpp>

#include <vector>

// https://github.com/KhronosGroup/Vulkan-Samples/tree/main/samples/api/hpp_hello_triangle

VkInstance vkInst = VK_NULL_HANDLE;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device = VK_NULL_HANDLE;

VkSwapchainKHR swapchain = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkSurfaceFormatKHR surfaceFormat;
std::vector<VkImageView> imageViews;

VkRenderPass renderPass;
VkPipeline pipeline;
std::vector<VkFramebuffer> framebuffers;

uint32_t graphicsQueueFamilyIndex = UINT32_MAX;

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_loadVulkan(JNIEnv *env, jobject obj)
{
    SDL_Vulkan_LoadLibrary(nullptr);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_destroy(JNIEnv *env, jobject obj)
{
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(vkInst, surface, nullptr);
    vkDestroyInstance(vkInst, nullptr);
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createInstance(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    SDL_Window *window = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    uint32_t extensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    std::vector<const char *> extensionNames;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames.data());

    const VkInstanceCreateInfo instInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        nullptr,
        0,
        nullptr,
        static_cast<uint32_t>(extensionNames.size()),
        extensionNames.data(),
    };

    VkResult result = vkCreateInstance(&instInfo, nullptr, &vkInst);
    if (result != VK_SUCCESS)
    {
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInstanceInitializationException");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("VkInstance initialization failed");
        jint jResult = static_cast<jint>(result);
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, jResult);
        env->Throw(static_cast<jthrowable>(exceptionObject));
        return;
    }

    if (!SDL_Vulkan_CreateSurface(window, vkInst, &surface))
    {
        // Creation of Surface failed
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
    vkEnumeratePhysicalDevices(vkInst, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        // No supported device found
        return;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInst, &deviceCount, devices.data());
    physicalDevice = devices[0];

    if (physicalDevice == VK_NULL_HANDLE)
    {
        // No supported device found
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInstanceInitializationException");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("No supported device found");
        jobject exceptionObject = env->NewObject(exceptionClass, constructorID, message, 0);
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
    std::vector<const char *> deviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkQueue graphicsQueue = VK_NULL_HANDLE;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

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
        // No suitable Grafik-Queue-Familie found
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

    VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        // Cannot create device
        jclass exceptionClass = env->FindClass("com/github/nodedev74/jfbx/exception/VkInstanceInitializationException");
        jmethodID constructorID = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
        jstring message = env->NewStringUTF("Unable to create VkDevice");
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
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createSwapchain(JNIEnv *env, jobject obj, jlong sdlWindowPtr)
{
    SDL_Window *window = reinterpret_cast<SDL_Window *>(sdlWindowPtr);

    if (surface == VK_NULL_HANDLE)
    {
        // Surface is not created or set
        return;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
    surfaceFormat = surfaceFormats[0];

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
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
        // Failed to create swapchain
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
            // Failed to create image view for the swapchain image
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        // Failed to create render pass
        return;
    }
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createGraphicsPipeline(JNIEnv *env, jobject obj)
{
}

/**
 * @brief
 *
 * @param env
 * @param obj
 */
JNIEXPORT void JNICALL Java_com_github_nodedev74_jfbx_vulkan_VkHandler_createFramebuffers(JNIEnv *env, jobject obj)
{
}