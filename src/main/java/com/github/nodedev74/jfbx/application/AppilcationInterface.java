package com.github.nodedev74.jfbx.application;

/**
 * Interface that provides methods to interact with.
 */
public interface AppilcationInterface {

    /**
     * Runs at application start.
     */
    public void start();

    /**
     * Runs at application stop.
     * By default there will be no stop action.
     */
    public default void stop() {
    }
}
