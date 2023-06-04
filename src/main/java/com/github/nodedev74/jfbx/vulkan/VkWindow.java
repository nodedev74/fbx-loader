package com.github.nodedev74.jfbx.vulkan;

import java.util.Timer;
import java.util.TimerTask;

import com.github.nodedev74.jfbx.stage.control.Control;

public class VkWindow extends Control {

    private long sdlWindowPtr;

    private Timer timer;
    private TimerTask timerTask;

    public VkWindow(int width, int height) {
        sdlWindowPtr = createSDLWindow(width, height);

        timer = new Timer();
        timerTask = new TimerTask() {
            public void run() {
                renderSDLWindow(sdlWindowPtr);
            }
        };
        timer.schedule(timerTask, 0, 16);
    }

    public void show() {
        showSDLWindow(sdlWindowPtr);
    }

    public void hide() {
        hideSDLWindow(sdlWindowPtr);
    }

    public void close() {
        timer.cancel();
        timerTask.cancel();
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
