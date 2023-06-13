package com.github.nodedev74.jfbx.application;

import java.util.ArrayList;
import java.util.Iterator;

import com.github.nodedev74.jfbx.NativeLoader;
import com.github.nodedev74.jfbx.stage.Stage;
import com.github.nodedev74.jfbx.stage.control.Control;

/**
 * 
 * Abstract class representing an application.
 * Provides methods for launching and managing the application lifecycle.
 */
public abstract class Application implements AppilcationInterface {

    protected static long FPS_TARGET = 30;

    private static boolean isRunning = true;

    public static Stage currentStage;

    /**
     * Launches the specified application class.
     * Loads the Vulkan library, creates a stage, and starts the application
     * lifecycle.
     * 
     * @param app the application class to launch
     */
    public static void launch(Class<? extends Application> app) {
        try {
            NativeLoader.load("libvulkan");
            currentStage = new Stage();
            Application application = app.getDeclaredConstructor().newInstance();

            application.start();
            Application.lifecycle();
            application.stop();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Exits the application.
     * Sets the isRunning flag to false.
     */
    public static void exit() {
        isRunning = false;
    }

    /**
     * Manages the application lifecycle.
     * Continuously processes the lifecycle of active controls in the current stage.
     * Exits the application when there are no more active controls.
     */
    private static void lifecycle() {
        long frameTime = 1000 / FPS_TARGET;
        long startTime = System.currentTimeMillis();

        while (isRunning) {
            ArrayList<? super Control> children = currentStage.children;
            if (!children.isEmpty()) {
                Iterator<? super Control> iterator = children.iterator();
                while (iterator.hasNext()) {
                    Control element = (Control) iterator.next();
                    if (element.isActive()) {
                        element.lifecycle();
                    } else {
                        iterator.remove();
                    }
                }
            } else {
                Application.exit();
            }

            long elapsedTime = System.currentTimeMillis() - startTime;
            long sleepTime = frameTime - elapsedTime;

            if (sleepTime > 0) {
                try {
                    Thread.sleep(sleepTime);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            startTime = System.currentTimeMillis();
        }
    }

}
