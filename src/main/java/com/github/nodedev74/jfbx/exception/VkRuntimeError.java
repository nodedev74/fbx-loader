package com.github.nodedev74.jfbx.exception;

public class VkRuntimeError extends RuntimeException {

    private int errorCode;

    public VkRuntimeError(String message) {
        super(message);
    }

    public VkRuntimeError(String message, int errorCode) {
        super(message);
        this.errorCode = errorCode;
    }
}
