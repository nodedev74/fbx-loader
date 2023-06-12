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

using namespace VkHelper;

/**
 * @brief Selects the presentation mode for a Vulkan surface.
 *
 * @param presentationModes The available presentation modes.
 * @param desiredPresentationMode The desired presentation mode.
 * @return The selected presentation mode.
 */
VkPresentModeKHR selectPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes, VkPresentModeKHR desiredPresentationMode)
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
uint32_t selectNumberOfImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities)
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
VkExtent2D selectSizeOfImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkExtent2D desiredSizeOfImages)
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
VkImageUsageFlags selectImageUsage(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkImageUsageFlags desiredUsages)
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
VkSurfaceTransformFlagBitsKHR selectSurfaceTransform(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkSurfaceTransformFlagBitsKHR desiredTransform)
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
VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats, VkSurfaceFormatKHR desiredSurfaceFormat)
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
uint32_t selectMemoryIndex(const VkPhysicalDeviceMemoryProperties &physicalDeviceMemoryProperties, const VkMemoryRequirements &memoryRequirements, VkMemoryPropertyFlagBits memoryProperties)
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
 * @brief Debug callback function for Vulkan validation layers.
 *
 * @param flags The flags associated with the debug report.
 * @param objType The type of the Vulkan object associated with the debug report.
 * @param srcObject The source object of the debug report.
 * @param location The location associated with the debug report.
 * @param msgCode The message code of the debug report.
 * @param pLayerPrefix The prefix of the validation layer.
 * @param pMsg The debug message.
 * @param pUserData User-defined data pointer.
 * @return VK_FALSE.
 */
VkBool32 debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        std::cerr << "ERROR: [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << std::endl;
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        std::cerr << "WARNING: [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << std::endl;
    }
    return VK_FALSE;
}