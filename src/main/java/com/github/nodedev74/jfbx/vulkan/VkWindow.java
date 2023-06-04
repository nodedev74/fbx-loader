package com.github.nodedev74.jfbx.vulkan;

import java.util.Timer;
import java.util.TimerTask;

import com.github.nodedev74.jfbx.stage.control.Control;

public class VkWindow extends Control {

    private long sdlWindowPtr;

    public VkWindow(int width, int height) {
        sdlWindowPtr = createSDLWindow(width, height);

        Timer timer = new Timer();
        timer.schedule(new TimerTask() {
            public void run() {
                renderSDLWindow(sdlWindowPtr);
            }
        }, 0, 16);
    }

    public void show() {
        showSDLWindow(sdlWindowPtr);
    }

    public void hide() {
        hideSDLWindow(sdlWindowPtr);
    }

    public void close() {
        destroySDLWindow(sdlWindowPtr);
        delete();
    }

    public long getWindowPtr() {
        return sdlWindowPtr;
    }

    @Override
    public void lifecycle() {
        cycleSDLWindow(sdlWindowPtr);
    }

    private native long createSDLWindow(int width, int height);

    private native void hideSDLWindow(long sdlWindowPtr);

    private native void showSDLWindow(long sdlWindowPtr);

    private native void renderSDLWindow(long sdlWindowPtr);

    private native void destroySDLWindow(long sdlWindowPtr);

    public native void cycleSDLWindow(long sdlWindowPtr);

}
