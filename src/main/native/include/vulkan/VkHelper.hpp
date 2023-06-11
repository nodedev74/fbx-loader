#ifndef VK_HELPER_HPP
#define VK_HELPER_HPP

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

namespace VkHelper
{
    /**
     * @brief
     *
     * @param presentation_modes
     * @param desired_presentation_mode
     * @return VkPresentModeKHR
     */
    VkPresentModeKHR selectPresentationMode(const std::vector<VkPresentModeKHR> &presentation_modes, VkPresentModeKHR desired_presentation_mode);

    /**
     * @brief
     *
     * @param surface_capabilities
     * @return uint32_t
     */
    uint32_t selectNumberOfImages(const VkSurfaceCapabilitiesKHR &surface_capabilities);

    /**
     * @brief
     *
     * @param surface_capabilities
     * @param desired_size_of_images
     * @return VkExtent2D
     */
    VkExtent2D selectSizeOfImages(const VkSurfaceCapabilitiesKHR &surface_capabilities, VkExtent2D desired_size_of_images);

    /**
     * @brief
     *
     * @param surface_capabilities
     * @param desired_usages
     * @return VkImageUsageFlags
     */
    VkImageUsageFlags selectImageUsage(const VkSurfaceCapabilitiesKHR &surface_capabilities, VkImageUsageFlags desired_usages);

    /**
     * @brief
     *
     * @param surface_capabilities
     * @param desired_transform
     * @return VkSurfaceTransformFlagBitsKHR
     */
    VkSurfaceTransformFlagBitsKHR selectSurfaceTransform(const VkSurfaceCapabilitiesKHR &surface_capabilities, VkSurfaceTransformFlagBitsKHR desired_transform);

    /**
     * @brief
     *
     * @param surface_formats
     * @param desired_surface_format
     * @return VkSurfaceFormatKHR
     */
    VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surface_formats, VkSurfaceFormatKHR desired_surface_format);

    /**
     * @brief
     *
     * @param physical_device_memory_properties
     * @param memory_requirements
     * @param memory_properties
     * @return uint32_t
     */
    uint32_t selectMemoryIndex(const VkPhysicalDeviceMemoryProperties &physical_device_memory_properties, const VkMemoryRequirements &memory_requirements, VkMemoryPropertyFlagBits memory_properties);

    /**
     * @brief
     *
     * @param flags
     * @param objType
     * @param srcObject
     * @param location
     * @param msgCode
     * @param pLayerPrefix
     * @param pMsg
     * @param pUserData
     * @return VkBool32
     */
    VkBool32 debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);
}

#endif // !VK_HELPER_HPP