package com.github.nodedev74.jfbx.vulkan;

public class VkHandler {

    private long sdlWindowPtr;

    /**
     * 
     * @param sdlWindowPtr
     */
    public VkHandler(long sdlWindowPtr) {
        this.sdlWindowPtr = sdlWindowPtr;
        this.prepare();
    }

    /**
     * 
     */
    private void prepare() {
        createInstance();
        selectPhysicalDevice();
        createDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
    }

    /**
     * 
     */
    public native void render();

    /**
     * 
     */
    public native void destroy();

    /**
     * 
     */
    private native void createInstance();

    /**
     * 
     */
    private native void selectPhysicalDevice();

    /**
     * 
     */
    private native void createDevice();

    /**
     * 
     */
    private native void createSwapchain();

    /**
     * 
     */
    private native void createImageViews();

    /**
     * 
     */
    private native void createRenderPass();

    /**
    * 
    */
    private native void createGraphicsPipeline();

    /**
     * 
     */
    private native void createFramebuffers();

    /**
     * 
     */
    private native void renderTriangle();

}
