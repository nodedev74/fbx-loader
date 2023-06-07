package com.github.nodedev74.jfbx.vulkan;

public class VkHandler {

    public VkHandler() {
        loadVulkan();
    }

    /**
     * 
     */
    private native void loadVulkan();

    /**
     * 
     */
    public native void destroy();

    /**
     * 
     */
    public native void createInstance(long sdlWindowPtr);

    /**
     * 
     */
    public native void selectPhysicalDevice();

    /**
     * 
     */
    public native void createLogicalDevice();

    /**
     * 
     * @param sdlWindowPtr
     */
    public native void createSwapchain(long sdlWindowPtr);

    /**
     * 
     */
    public native void createImageViews();

    /**
     * 
     */
    public native void createRenderPass();

    /**
     * 
     */
    public native void createGraphicsPipeline();

    /**
     * 
     */
    public native void createFramebuffers();

}
