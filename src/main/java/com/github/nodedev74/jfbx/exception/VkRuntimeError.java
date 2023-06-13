package com.github.nodedev74.jfbx.exception;

/**
 * Runtime error thrown by Vulkan.
 */
public class VkRuntimeError extends RuntimeException {

    /**
     * Constructs a Vulkan runtime exception.
     * 
     * @param message The message to be thrown.
     */
    public VkRuntimeError(String message) {
        super(message);
    }
}
