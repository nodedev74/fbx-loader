package com.github.nodedev74.jfbx.exception;

public class VkRuntimeError extends RuntimeException {

    public VkRuntimeError(String message) {
        super(message);
    }

    public VkRuntimeError(String message, int errorCode) {
        super(message + "\nERROR_CODE: " + errorCode);
    }
}
