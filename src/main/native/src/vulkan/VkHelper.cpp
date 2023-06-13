/**
 * @file VkHelper.cpp
 * @author Lenard Büsing (nodedev74@gmail.com)
 * @brief Contains helper functions for Vulkan operations.
 * @version 0.1
 * @date 2023-06-12
 *
 * @copyright Copyright (c) 2023 Lenard Büsing
 *
 */

#include "vulkan/VkHelper.hpp"

/**
 * @brief Selects the presentation mode for a Vulkan surface.
 *
 * @param presentationModes The available presentation modes.
 * @param desiredPresentationMode The desired presentation mode.
 * @return The selected presentation mode.
 */
VkPresentModeKHR VkHelper::selectPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes, VkPresentModeKHR desiredPresentationMode)
{
    VkPresentModeKHR selectedPresentMode;
    if (std::find(presentationModes.begin(), presentationModes.end(), desiredPresentationMode) != presentationModes.end())
    {
        selectedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else
    {
        selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    return selectedPresentMode;
}

/**
 * @brief Selects the number of images for a Vulkan surface.
 *
 * @param surfaceCapabilities The capabilities of the surface.
 * @return The selected number of images.
 */
uint32_t VkHelper::selectNumberOfImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities)
{
    uint32_t numberOfImages = surfaceCapabilities.minImageCount + 1;
    if ((surfaceCapabilities.maxImageCount > 0) && (numberOfImages > surfaceCapabilities.maxImageCount))
    {
        numberOfImages = surfaceCapabilities.maxImageCount;
    }
    return numberOfImages;
}

/**
 * @brief Selects the size of images for a Vulkan surface.
 *
 * @param surfaceCapabilities The capabilities of the surface.
 * @param desiredSizeOfImages The desired size of the images.
 * @return The selected size of images.
 */
VkExtent2D VkHelper::selectSizeOfImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkExtent2D desiredSizeOfImages)
{
    if (0xFFFFFFFF == surfaceCapabilities.currentExtent.width)
    {

        if (desiredSizeOfImages.width < surfaceCapabilities.minImageExtent.width)
        {
            desiredSizeOfImages.width = surfaceCapabilities.minImageExtent.width;
        }
        else if (desiredSizeOfImages.width > surfaceCapabilities.maxImageExtent.width)
        {
            desiredSizeOfImages.width = surfaceCapabilities.maxImageExtent.width;
        }

        if (desiredSizeOfImages.height < surfaceCapabilities.minImageExtent.height)
        {
            desiredSizeOfImages.height = surfaceCapabilities.minImageExtent.height;
        }
        else if (desiredSizeOfImages.height > surfaceCapabilities.maxImageExtent.height)
        {
            desiredSizeOfImages.height = surfaceCapabilities.maxImageExtent.height;
        }
    }
    else
    {
        desiredSizeOfImages = surfaceCapabilities.currentExtent;
    }
    return desiredSizeOfImages;
}

/**
 * @brief Selects the image usage flags for a Vulkan surface.
 *
 * @param surfaceCapabilities The capabilities of the surface.
 * @param desiredUsages The desired image usage flags.
 * @return The selected image usage flags.
 */
VkImageUsageFlags VkHelper::selectImageUsage(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkImageUsageFlags desiredUsages)
{
    VkImageUsageFlags imageUsage;
    imageUsage = desiredUsages & surfaceCapabilities.supportedUsageFlags;
    if (desiredUsages != imageUsage)
    {
        imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    return imageUsage;
}

/**
 * @brief Selects the surface transform flag bits for a Vulkan surface.
 *
 * @param surfaceCapabilities The capabilities of the surface.
 * @param desiredTransform The desired surface transform flag bits.
 * @return The selected surface transform flag bits.
 */
VkSurfaceTransformFlagBitsKHR VkHelper::selectSurfaceTransform(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkSurfaceTransformFlagBitsKHR desiredTransform)
{
    VkSurfaceTransformFlagBitsKHR surfaceTransform;
    if (surfaceCapabilities.supportedTransforms & desiredTransform)
    {
        surfaceTransform = desiredTransform;
    }
    else
    {
        surfaceTransform = surfaceCapabilities.currentTransform;
    }
    return surfaceTransform;
}

/**
 * @brief Selects the surface format for a Vulkan surface.
 *
 * @param surfaceFormats The available surface formats.
 * @param desiredSurfaceFormat The desired surface format.
 * @return The selected surface format.
 */
VkSurfaceFormatKHR VkHelper::selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats, VkSurfaceFormatKHR desiredSurfaceFormat)
{
    VkSurfaceFormatKHR selectedSurfaceFormat;
    if ((1 == surfaceFormats.size()) &&
        (VK_FORMAT_UNDEFINED == surfaceFormats[0].format))
    {
        selectedSurfaceFormat.format = desiredSurfaceFormat.format;
        selectedSurfaceFormat.colorSpace = desiredSurfaceFormat.colorSpace;
        return selectedSurfaceFormat;
    }

    for (auto &surfaceFormat : surfaceFormats)
    {
        if ((desiredSurfaceFormat.format == surfaceFormat.format) &&
            (desiredSurfaceFormat.colorSpace == surfaceFormat.colorSpace))
        {
            selectedSurfaceFormat.format = desiredSurfaceFormat.format;
            selectedSurfaceFormat.colorSpace = desiredSurfaceFormat.colorSpace;
            return surfaceFormat;
        }
    }

    for (auto &surfaceFormat : surfaceFormats)
    {
        if ((desiredSurfaceFormat.format == surfaceFormat.format))
        {
            selectedSurfaceFormat.format = desiredSurfaceFormat.format;
            selectedSurfaceFormat.colorSpace = desiredSurfaceFormat.colorSpace;
            return selectedSurfaceFormat;
        }
    }

    selectedSurfaceFormat = surfaceFormats[0];
    return selectedSurfaceFormat;
}

/**
 * @brief Selects the memory index for a Vulkan memory allocation.
 *
 * @param physicalDeviceMemoryProperties The properties of the physical device memory.
 * @param memoryRequirements The memory requirements for the allocation.
 * @param memoryProperties The desired memory property flags.
 * @return The selected memory index.
 */
uint32_t VkHelper::selectMemoryIndex(const VkPhysicalDeviceMemoryProperties &physicalDeviceMemoryProperties, const VkMemoryRequirements &memoryRequirements, VkMemoryPropertyFlagBits memoryProperties)
{
    for (uint32_t type = 0; type < physicalDeviceMemoryProperties.memoryTypeCount; ++type)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << type)) &&
            ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & memoryProperties) == memoryProperties))
        {
            return type;
        }
    }
    return VK_MAX_MEMORY_TYPES;
}

/**
 * @brief The callback
 *
 * @param messageSeverity The severity of the message.
 * @param messageType The type of the message.
 * @param pCallbackData The callback data containing information about the message.
 * @param pUserData User-defined data pointer.
 * @return VkBool32 A value indicating whether the execution of the Vulkan function should continue.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL VkHelper::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    validationOutputFile << pCallbackData->pMessage << std::endl;

    std::cout << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}