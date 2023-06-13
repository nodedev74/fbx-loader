package com.github.nodedev74.jfbx.vulkan;

import com.github.nodedev74.jfbx.stage.control.Control;

/**
 * Vulkan window based on SDLWindow
 */
public class VkWindow extends Control {

    private long sdlWindowPtr; // SDK_Window*

    private int width;
    private int height;

    private VkHandler handler;

    /**
     * Constructs a Vulkan window with its Vulkan handler
     * 
     * @param width  The width of the window.
     * @param height The height of the window.
     */
    public VkWindow(int width, int height) {
        this.width = width;
        this.height = height;

        sdlWindowPtr = create(width, height);
        handler = new VkHandler(sdlWindowPtr);
    }

    /**
     * JNI function to create a Vulkan window.
     *
     * @param width  The width of the window.
     * @param height The height of the window.
     * @return The pointer to the created SDLWindow as a jlong value.
     */
    public native long create(int width, int height);

    /**
     * JNI function to destroy a Vulkan window.
     */
    public native void destroy();

    /**
     * JNI function to show a Vulkan window.
     */
    public native void show();

    /**
     * JNI function to hide a Vulkan window.
     */
    public native void hide();

    /**
     * JNI function run the lifecycle of the SDLWindow
     * 
     * @param sdlWindowPtr The pointer to the SDLWindow as a jlong value.
     */
    private native void run(long sdlWindowPtr);

    /**
     * Executes the lifecycle of the Vulkan window.
     * This method renders the Vulkan window using the handler, and handles SDL
     * events.
     */
    @Override
    public void lifecycle() {
        handler.render();
        run(sdlWindowPtr);
    }
}
