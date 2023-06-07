package com.github.nodedev74.jfbx.vulkan;

import com.github.nodedev74.jfbx.stage.control.Control;

public class VkWindow extends Control {

    private long sdlWindowPtr;

    private int width;
    private int height;

    private VkHandler handler;
    private VkRenderer renderer;

    public VkWindow(int width, int height) {
        this.width = width;
        this.height = height;

        sdlWindowPtr = create(width, height);
        handler = new VkHandler(sdlWindowPtr);
    }

    /**
     * 
     * @param width
     * @param height
     * @return
     */
    public native long create(int width, int height);

    /**
     * 
     */
    public native void destroy();

    /**
     * 
     */
    public native void show();

    /**
     * 
     */
    public native void hide();

    /**
     * 
     * @param sdlWindowPtr
     */
    private native void run(long sdlWindowPtr);

    @Override
    public void lifecycle() {
        run(sdlWindowPtr);
    }
}
