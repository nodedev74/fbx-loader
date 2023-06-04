package com.github.nodedev74.jfbx.application;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Iterator;

import com.github.nodedev74.jfbx.NativeLoader;
import com.github.nodedev74.jfbx.stage.Stage;
import com.github.nodedev74.jfbx.stage.control.Control;

public abstract class Application implements AppilcationInterface {

    private static boolean isRunning = true;

    public static Stage currentStage;

    public static void launch(Class<? extends Application> app) {
        NativeLoader.load("libhello");
        try {
            currentStage = new Stage();
            Application application = app.getDeclaredConstructor().newInstance();

            application.start();
            Application.lifecycle();
            application.stop();
        } catch (InstantiationException | IllegalAccessException | IllegalArgumentException | InvocationTargetException
                | NoSuchMethodException | SecurityException e) {
            e.printStackTrace();
        }
    }

    public static void exit() {
        isRunning = false;
    }

    private static void lifecycle() {
        while (isRunning) {
            ArrayList<? super Control> children = currentStage.children;
            if (children.size() != 0) {
                Iterator<? super Control> iterator = children.iterator();
                while (iterator.hasNext()) {
                    Object child = iterator.next();
                    Control element = (Control) child;
                    if (element.isActive()) {
                        element.lifecycle();
                    } else {
                        iterator.remove();
                        if (children.size() == 0) {
                            Application.exit();
                        }
                    }
                }
            } else {
                Application.exit();
            }
            try {
                Thread.sleep(34);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

}
