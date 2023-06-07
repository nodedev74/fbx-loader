package com.github.nodedev74.jfbx.exception;

public class VkInstanceInitializationException extends VkRuntimeException {
    public VkInstanceInitializationException(String message, int vkResult) {
        super(message, vkResult);
    }
}
