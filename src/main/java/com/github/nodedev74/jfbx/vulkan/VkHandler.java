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
        createDebugger();
        createSureface();
        createLogicalDevice();
        createSwapchain();
        createCommandPool();
        allocateCommandBuffers();
        createHostBuffers();
        createDeviceBuffers();
        createDescriptorPool();
        allocateDescriptorSets();
        createRenderpass();
        createFramebuffers();
        createPipeline();
        uploadInputData();
        recordCommandBuffers();
        createSemaphores();
    }

    /**
     * 
     */
    private native void createInstance();

    /**
     * 
     */
    private native void createDebugger();

    /**
     * 
     */
    private native void createSureface();

    /**
     * 
     */
    private native void createLogicalDevice();

    /**
     * 
     */
    private native void createSwapchain();

    /**
     * 
     */
    private native void createCommandPool();

    /**
     * 
     */
    private native void allocateCommandBuffers();

    /**
     * 
     */
    private native void createHostBuffers();

    /**
     * 
     */
    private native void createDeviceBuffers();

    /**
     * 
     */
    private native void createDescriptorPool();

    /**
     * 
     */
    private native void allocateDescriptorSets();

    /**
     * 
     */
    private native void createRenderpass();

    /**
     * 
     */
    private native void createFramebuffers();

    /**
     * 
     */
    private native void createPipeline();

    /**
     * 
     */
    private native void uploadInputData();

    /**
     * 
     */
    private native void recordCommandBuffers();

    /**
     * 
     */
    private native void createSemaphores();

    /**
     * 
     */
    public native void render();

    /**
     * 
     */
    public native void destroy();
}
