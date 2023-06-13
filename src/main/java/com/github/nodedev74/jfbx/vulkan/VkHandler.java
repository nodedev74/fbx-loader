package com.github.nodedev74.jfbx.vulkan;

/**
 * Contains interaction layer with Vulkan.
 */
public class VkHandler {

    private long sdlWindowPtr;

    /**
     * Constructs a Vulkan handler and prepares it
     * 
     * @param sdlWindowPtr The pointer to the created SDLWindow as a jlong value.
     */
    public VkHandler(long sdlWindowPtr) {
        this.sdlWindowPtr = sdlWindowPtr;
        this.prepare();
    }

    /**
     * Prepares Vulkan to get ready for render
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
     * Creates a Vulkan instance.
     */
    private native void createInstance();

    /**
     * Creates a Vulkan debugger
     */
    private native void createDebugger();

    /**
     * Creates a Vulkan surface for SDLWindow
     */
    private native void createSureface();

    /**
     * Creates a logical Vulkan device
     */
    private native void createLogicalDevice();

    /**
     * Creates a Vulkan swapchain
     */
    private native void createSwapchain();

    /**
     * Creates a Vulkan command pool
     */
    private native void createCommandPool();

    /**
     * Allocates the command buffers
     */
    private native void allocateCommandBuffers();

    /**
     * Creates host buffers
     */
    private native void createHostBuffers();

    /**
     * Creates device buffers
     */
    private native void createDeviceBuffers();

    /**
     * Creates Descriptor pool
     */
    private native void createDescriptorPool();

    /**
     * Allocates Descriptor Sets
     */
    private native void allocateDescriptorSets();

    /**
     * Creates a Vulkan Renderpass
     */
    private native void createRenderpass();

    /**
     * Creates Vulkan Framebuffers
     */
    private native void createFramebuffers();

    /**
     * Creates a Vulkan graphics pipeline
     */
    private native void createPipeline();

    /**
     * Uploads the input data
     */
    private native void uploadInputData();

    /**
     * Records the command buffers for Vulkan rendering.
     */
    private native void recordCommandBuffers();

    /**
     * Creates semaphores for Vulkan synchronization.
     */
    private native void createSemaphores();

    /**
     * Renders the Vulkan scene.
     */
    public native void render();

    /**
     * Destroys the Vulkan resources.
     */
    public native void destroy();
}
