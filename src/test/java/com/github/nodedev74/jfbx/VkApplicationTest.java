package com.github.nodedev74.jfbx;

import com.github.nodedev74.jfbx.application.Application;
import com.github.nodedev74.jfbx.vulkan.VkWindow;

public class VkApplicationTest extends Application {

    @Override
    public void start() {
        VkWindow vkWindow = new VkWindow(800, 800);
        vkWindow.show();
        Application.currentStage.addChildren(vkWindow);
    }
}
