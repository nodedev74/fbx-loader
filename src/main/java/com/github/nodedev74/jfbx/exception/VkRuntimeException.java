package com.github.nodedev74.jfbx.exception;

public class VkRuntimeException extends RuntimeException {
    public VkRuntimeException(String message, int vkResult) {
        super(message + "ERROR_CODE: " + vkResult);
    }

}
