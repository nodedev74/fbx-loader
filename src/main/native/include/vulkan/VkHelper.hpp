/**
 * @file VkHelper.hpp
 * @author Lenard Büsing (nodedev74@gmail.com)
 * @brief Contains helper functions for Vulkan operations.
 * @version 0.1
 * @date 2023-06-12
 *
 * @copyright Copyright (c) 2023 Lenard Büsing
 *
 */

#ifndef VK_HELPER_HPP
#define VK_HELPER_HPP

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.hpp>

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

namespace VkHelper
{
    /**
     * @brief Selects the presentation mode for a Vulkan surface.
     *
     * @param presentationModes The available presentation modes.
     * @param desiredPresentationMode The desired presentation mode.
     * @return The selected presentation mode.
     */
    VkPresentModeKHR selectPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes, VkPresentModeKHR desiredPresentationMode);

    /**
     * @brief Selects the number of images for a Vulkan surface.
     *
     * @param surfaceCapabilities The capabilities of the surface.
     * @return The selected number of images.
     */
    uint32_t selectNumberOfImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);

    /**
     * @brief Selects the size of images for a Vulkan surface.
     *
     * @param surfaceCapabilities The capabilities of the surface.
     * @param desiredSizeOfImages The desired size of the images.
     * @return The selected size of images.
     */
    VkExtent2D selectSizeOfImages(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkExtent2D desiredSizeOfImages);

    /**
     * @brief Selects the image usage flags for a Vulkan surface.
     *
     * @param surfaceCapabilities The capabilities of the surface.
     * @param desiredUsages The desired image usage flags.
     * @return The selected image usage flags.
     */
    VkImageUsageFlags selectImageUsage(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkImageUsageFlags desiredUsages);

    /**
     * @brief Selects the surface transform flag bits for a Vulkan surface.
     *
     * @param surfaceCapabilities The capabilities of the surface.
     * @param desiredTransform The desired surface transform flag bits.
     * @return The selected surface transform flag bits.
     */
    VkSurfaceTransformFlagBitsKHR selectSurfaceTransform(const VkSurfaceCapabilitiesKHR &surfaceCapabilities, VkSurfaceTransformFlagBitsKHR desiredTransform);

    /**
     * @brief Selects the surface format for a Vulkan surface.
     *
     * @param surfaceFormats The available surface formats.
     * @param desiredSurfaceFormat The desired surface format.
     * @return The selected surface format.
     */
    VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats, VkSurfaceFormatKHR desiredSurfaceFormat);

    /**
     * @brief Selects the memory index for a Vulkan memory allocation.
     *
     * @param physicalDeviceMemoryProperties The properties of the physical device memory.
     * @param memoryRequirements The memory requirements for the allocation.
     * @param memoryProperties The desired memory property flags.
     * @return The selected memory index.
     */
    uint32_t selectMemoryIndex(const VkPhysicalDeviceMemoryProperties &physicalDeviceMemoryProperties, const VkMemoryRequirements &memoryRequirements, VkMemoryPropertyFlagBits memoryProperties);

    /**
     * @brief The callback
     *
     * @param messageSeverity The severity of the message.
     * @param messageType The type of the message.
     * @param pCallbackData The callback data containing information about the message.
     * @param pUserData User-defined data pointer.
     * @return VkBool32 A value indicating whether the execution of the Vulkan function should continue.
     */
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);
}

#endif // !VK_HELPER_HPP