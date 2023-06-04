package com.github.nodedev74.jfbx;

import com.github.nodedev74.jfbx.application.Application;
import com.github.nodedev74.jfbx.vulkan.VkWindow;

public class VkApplicationTest extends Application {

    @Override
    public void start() {
        VkWindow win = new VkWindow(800, 800);
        win.show();

        Application.currentStage.children.add(win);
    }
}
